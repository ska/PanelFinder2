#!/usr/bin/env python3
"""
UdpFinder Simulator

Simula N pannelli che rispondono al protocollo UdpFinder:
  - Ascolta broadcast "WhereAreYou.02" sulla porta 991
  - Ogni pannello ha un IP alias sull'interfaccia di rete
  - Ogni pannello risponde con il proprio IP sorgente (come fanno i dispositivi reali)

struct_selfinfo1 layout (char arrays, nessun padding):
  identification[10]        "I am here."
  opcode[2]                 "02"
  hardware[16]              MAC string (12 hex chars senza separatori)
  ip[16]                    IP string
  netmask[16]               netmask string
  RemoteConfiguratorPort[4] porta
  moduleName[5]             nome modulo (space-padded: trimmed() in Qt rimuove spazi, non i \x00)
  manufactureCode[2]        \x00\x00
  reserved[12]              \x00 * 12
  hostName[64]              hostname
  Total: 147 + 1 byte padding = 148 → hostname parsato dal client

Utilizzo:
  sudo python3 udpfinder_simulator.py                        # 5 pannelli, interfaccia auto
  sudo python3 udpfinder_simulator.py -n 10                  # 10 pannelli
  sudo python3 udpfinder_simulator.py -n 5 -i eth0           # interfaccia specifica
  python3 udpfinder_simulator.py --list                      # mostra pannelli ed esce

NOTA: richiede root per bind su porta 991 e per aggiungere IP alias.
      Gli IP alias vengono rimossi automaticamente all'uscita.
"""

import socket
import struct
import random
import time
import argparse
import subprocess
import atexit
import sys


LISTEN_PORT  = 991
STRUCT_FMT   = "10s2s16s16s16s4s5s2s12s64s"
STRUCT_SIZE  = struct.calcsize(STRUCT_FMT)   # 147

MODULE_NAMES = ["UN67", "UN68", "UN83", "UN84"]

_added_aliases = []   # lista (ip, iface) da rimuovere all'uscita


# ---------------------------------------------------------------------------
# IP alias management
# ---------------------------------------------------------------------------

def get_default_interface() -> str:
    """Rileva l'interfaccia di rete predefinita."""
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
    """Aggiunge un IP alias sull'interfaccia. Restituisce True se OK."""
    result = subprocess.run(
        ["ip", "addr", "add", f"{ip}/24", "dev", interface],
        capture_output=True
    )
    if result.returncode == 0:
        _added_aliases.append((ip, interface))
        return True
    if b"File exists" in result.stderr:
        return True  # già presente, non aggiungere alla lista di cleanup
    print(f"WARN: impossibile aggiungere alias {ip} su {interface}: {result.stderr.decode().strip()}")
    return False


def remove_ip_aliases():
    """Rimuove tutti gli IP alias aggiunti da questa sessione."""
    for ip, iface in _added_aliases:
        subprocess.run(
            ["ip", "addr", "del", f"{ip}/24", "dev", iface],
            capture_output=True
        )

atexit.register(remove_ip_aliases)


# ---------------------------------------------------------------------------
# Panel data
# ---------------------------------------------------------------------------

class Panel:
    def __init__(self):
        self.ip       = f"192.168.1.{random.randint(10, 250)}"
        self.mac      = "".join(f"{random.randint(0, 255):02x}" for _ in range(6))
        self.hostname = f"Panel-{random.randint(1000, 9999)}"
        self.module   = random.choice(MODULE_NAMES)
        self.netmask  = "255.255.255.0"
        self.sock     = None  # socket dedicato, bind sul proprio IP

    def __str__(self):
        return f"{self.hostname:20s}  {self.ip:16s}  {self.mac}  {self.module}"


def generate_panels(count: int) -> list:
    panels, used_ips = [], set()
    for _ in range(count):
        p = Panel()
        while p.ip in used_ips:
            p.ip = f"192.168.1.{random.randint(10, 250)}"
        used_ips.add(p.ip)
        panels.append(p)
    return panels


# ---------------------------------------------------------------------------
# Packet building
# ---------------------------------------------------------------------------

def _field(value: str, size: int) -> bytes:
    # Space-padding: Qt::trimmed() rimuove spazi ma NON i byte null (\x00)
    return value.encode("ascii")[:size].ljust(size, b" ")


def build_response(panel: Panel) -> bytes:
    data = struct.pack(
        STRUCT_FMT,
        b"I am here.",
        b"02",
        _field(panel.mac,      16),
        _field(panel.ip,       16),
        _field(panel.netmask,  16),
        b"9999",
        _field(panel.module,    5),
        b"\x00\x00",
        b"\x00" * 12,
        _field(panel.hostname, 64),
    )
    return data + b"\x00"   # pad a 148 → hostname parsato dal client


# ---------------------------------------------------------------------------
# Main loop
# ---------------------------------------------------------------------------

def run(panels: list, interface: str):
    # Socket di ascolto sulla porta 991
    listen_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    listen_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listen_sock.settimeout(1.0)
    try:
        listen_sock.bind(("", LISTEN_PORT))
    except PermissionError:
        print(f"ERRORE: impossibile fare bind sulla porta {LISTEN_PORT} — eseguire con sudo")
        sys.exit(1)

    # Crea IP alias e socket dedicato per ogni pannello
    print(f"Interfaccia: {interface}")
    for panel in panels:
        if not add_ip_alias(panel.ip, interface):
            print(f"ERRORE: impossibile aggiungere alias per {panel.ip}")
            sys.exit(1)
        panel.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        panel.sock.bind((panel.ip, 0))  # risponde CON l'IP del pannello
        print(f"  + alias {panel.ip} ({panel.hostname})")

    print(f"\nIn ascolto sulla porta {LISTEN_PORT}...\n")

    try:
        while True:
            try:
                data, addr = listen_sock.recvfrom(256)
            except socket.timeout:
                continue

            if data == b"WhereAreYou.02":
                print(f"Discovery da {addr[0]}:{addr[1]}")
                for panel in panels:
                    panel.sock.sendto(build_response(panel), addr)
                    print(f"  → {panel.hostname} ({panel.ip})")
                    time.sleep(0.01)
    except KeyboardInterrupt:
        print("\nStop.")
    finally:
        listen_sock.close()
        for panel in panels:
            if panel.sock:
                panel.sock.close()


def main():
    parser = argparse.ArgumentParser(description="UdpFinder simulator")
    parser.add_argument("-n", "--num-panels", type=int, default=5,
                        help="numero pannelli da simulare (default: 5)")
    parser.add_argument("-i", "--interface", type=str, default=None,
                        help="interfaccia di rete per gli IP alias (default: auto)")
    parser.add_argument("--list", action="store_true",
                        help="mostra pannelli ed esce")
    args = parser.parse_args()

    interface = args.interface or get_default_interface()
    panels    = generate_panels(args.num_panels)

    print(f"\nPannelli simulati ({len(panels)}):")
    print("─" * 62)
    for p in panels:
        print(f"  {p}")
    print("─" * 62 + "\n")

    if args.list:
        return

    run(panels, interface)


if __name__ == "__main__":
    main()
