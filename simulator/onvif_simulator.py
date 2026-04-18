#!/usr/bin/env python3
"""
ONVIF WS-Discovery Simulator

Simula N telecamere che rispondono al protocollo ONVIF WS-Discovery.

Per ogni telecamera simulata:
  - Aggiunge un IP alias sull'interfaccia di rete  → IP sorgente univoco
  - Aggiunge una voce ARP statica                  → getMacForIP() funziona
  - Crea un socket dedicato bind sull'IP della camera

IP alias e voci ARP vengono rimossi automaticamente all'uscita.

Utilizzo:
  sudo python3 onvif_simulator.py                  # 5 telecamere, interfaccia auto
  sudo python3 onvif_simulator.py -n 10            # 10 telecamere
  sudo python3 onvif_simulator.py -n 5 -i eth0     # interfaccia specifica
  python3 onvif_simulator.py --list                # mostra telecamere ed esce

NOTA: richiede root per aggiungere IP alias e voci ARP.
"""

import socket
import struct
import random
import time
import uuid
import re
import argparse
import subprocess
import atexit
import sys


MCAST_GRP  = "239.255.255.250"
MCAST_PORT = 3702

_added_aliases = []   # (ip, iface) da rimuovere
_added_neigh   = []   # (ip, iface) da rimuovere


# ---------------------------------------------------------------------------
# Network setup / teardown
# ---------------------------------------------------------------------------

def get_default_interface() -> str:
    try:
        result = subprocess.run(
            ["ip", "route", "get", "8.8.8.8"],
            capture_output=True, text=True, check=True
        )
        tokens = result.stdout.split()
        if "dev" in tokens:
            return tokens[tokens.index("dev") + 1]
    except Exception:
        pass
    return "eth0"


def add_ip_alias(ip: str, interface: str) -> bool:
    result = subprocess.run(
        ["ip", "addr", "add", f"{ip}/24", "dev", interface],
        capture_output=True
    )
    if result.returncode == 0:
        _added_aliases.append((ip, interface))
        return True
    if b"File exists" in result.stderr:
        return True
    print(f"WARN: impossibile aggiungere alias {ip} su {interface}: {result.stderr.decode().strip()}")
    return False


def add_arp_entry(ip: str, mac: str, interface: str) -> bool:
    """Aggiunge voce ARP statica → getMacForIP() in PanelFinder2 trova il MAC."""
    mac_fmt = ":".join(mac[i:i+2] for i in range(0, 12, 2))
    result = subprocess.run(
        ["ip", "neigh", "add", ip, "lladdr", mac_fmt,
         "dev", interface, "nud", "permanent"],
        capture_output=True
    )
    if result.returncode == 0:
        _added_neigh.append((ip, interface))
        return True
    if b"File exists" in result.stderr:
        # Aggiorna voce esistente
        subprocess.run(
            ["ip", "neigh", "replace", ip, "lladdr", mac_fmt,
             "dev", interface, "nud", "permanent"],
            capture_output=True
        )
        _added_neigh.append((ip, interface))
        return True
    print(f"WARN: impossibile aggiungere ARP entry {ip}: {result.stderr.decode().strip()}")
    return False


def cleanup():
    for ip, iface in _added_neigh:
        subprocess.run(["ip", "neigh", "del", ip, "dev", iface], capture_output=True)
    for ip, iface in _added_aliases:
        subprocess.run(["ip", "addr", "del", f"{ip}/24", "dev", iface], capture_output=True)

atexit.register(cleanup)


# ---------------------------------------------------------------------------
# Camera data
# ---------------------------------------------------------------------------

class Camera:
    def __init__(self):
        self.ip       = f"192.168.1.{random.randint(10, 250)}"
        self.mac      = "".join(f"{random.randint(0, 255):02x}" for _ in range(6))
        self.hostname = f"Camera-{random.randint(1000, 9999)}"
        self.sock     = None  # socket dedicato bind sul proprio IP

    def __str__(self):
        mac_fmt = ":".join(self.mac[i:i+2] for i in range(0, 12, 2))
        return f"{self.hostname:20s}  {self.ip:16s}  {mac_fmt}"


def generate_cameras(count: int) -> list:
    cameras, used_ips = [], set()
    for _ in range(count):
        c = Camera()
        while c.ip in used_ips:
            c.ip = f"192.168.1.{random.randint(10, 250)}"
        used_ips.add(c.ip)
        cameras.append(c)
    return cameras


# ---------------------------------------------------------------------------
# ONVIF packet
# ---------------------------------------------------------------------------

def build_probe_match(camera: Camera, relates_to: str) -> bytes:
    xml = (
        '<?xml version="1.0" encoding="utf-8"?>'
        '<e:Envelope xmlns:e="http://www.w3.org/2003/05/soap-envelope" '
        'xmlns:w="http://schemas.xmlsoap.org/ws/2004/08/addressing" '
        'xmlns:d="http://schemas.xmlsoap.org/ws/2005/04/discovery" '
        'xmlns:dn="http://www.onvif.org/ver10/network/wsdl">'
        "<e:Header>"
        f"<w:MessageID>uuid:{uuid.uuid4()}</w:MessageID>"
        f"<w:RelatesTo>{relates_to}</w:RelatesTo>"
        "<w:To>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</w:To>"
        "<w:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</w:Action>"
        "</e:Header>"
        "<e:Body>"
        "<d:ProbeMatches>"
        "<d:ProbeMatch>"
        f"<w:EndpointReference><w:Address>urn:uuid:{uuid.uuid4()}</w:Address></w:EndpointReference>"
        "<d:Types>dn:NetworkVideoTransmitter</d:Types>"
        f"<d:Scopes>onvif://www.onvif.org/name/{camera.hostname}</d:Scopes>"
        f"<d:XAddrs>http://{camera.ip}/onvif/device_service</d:XAddrs>"
        "<d:MetadataVersion>1</d:MetadataVersion>"
        "</d:ProbeMatch>"
        "</d:ProbeMatches>"
        "</e:Body>"
        "</e:Envelope>"
    )
    return xml.encode("utf-8")


# ---------------------------------------------------------------------------
# Main loop
# ---------------------------------------------------------------------------

def run(cameras: list, interface: str):
    # Setup IP alias + ARP entry + socket per ogni camera
    print(f"Interfaccia: {interface}")
    for camera in cameras:
        if not add_ip_alias(camera.ip, interface):
            print(f"ERRORE: impossibile aggiungere alias per {camera.ip}")
            sys.exit(1)
        if not add_arp_entry(camera.ip, camera.mac, interface):
            print(f"ERRORE: impossibile aggiungere ARP entry per {camera.ip}")
            sys.exit(1)
        camera.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        camera.sock.bind((camera.ip, 0))  # risponde CON l'IP della camera
        mac_fmt = ":".join(camera.mac[i:i+2] for i in range(0, 12, 2))
        print(f"  + alias {camera.ip}  MAC {mac_fmt}  ({camera.hostname})")

    # Socket di ascolto multicast
    recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    recv_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    recv_sock.settimeout(1.0)
    recv_sock.bind(("", MCAST_PORT))
    mreq = struct.pack("4sL", socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)
    recv_sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    print(f"\nIn ascolto su multicast {MCAST_GRP}:{MCAST_PORT}...\n")

    try:
        while True:
            try:
                data, addr = recv_sock.recvfrom(4096)
            except socket.timeout:
                continue

            xml = data.decode("utf-8", errors="ignore")
            if "Probe" not in xml or "NetworkVideoTransmitter" not in xml:
                continue

            m = re.search(r"<w:MessageID>(.*?)</w:MessageID>", xml)
            relates_to = m.group(1) if m else f"uuid:{uuid.uuid4()}"

            print(f"Probe da {addr[0]}:{addr[1]}")
            for camera in cameras:
                time.sleep(random.uniform(0.05, 0.15))
                camera.sock.sendto(build_probe_match(camera, relates_to), addr)
                print(f"  → {camera.hostname} ({camera.ip})")
    except KeyboardInterrupt:
        print("\nStop.")
    finally:
        recv_sock.close()
        for camera in cameras:
            if camera.sock:
                camera.sock.close()


def main():
    parser = argparse.ArgumentParser(description="ONVIF WS-Discovery simulator")
    parser.add_argument("-n", "--num-panels", type=int, default=5,
                        help="numero telecamere da simulare (default: 5)")
    parser.add_argument("-i", "--interface", type=str, default=None,
                        help="interfaccia di rete (default: auto)")
    parser.add_argument("--list", action="store_true",
                        help="mostra telecamere ed esce")
    args = parser.parse_args()

    interface = args.interface or get_default_interface()
    cameras   = generate_cameras(args.num_panels)

    print(f"\nTelecamere simulate ({len(cameras)}):")
    print("─" * 62)
    for c in cameras:
        print(f"  {c}")
    print("─" * 62 + "\n")

    if args.list:
        return

    run(cameras, interface)


if __name__ == "__main__":
    main()
