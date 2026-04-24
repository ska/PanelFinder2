#include "newstonkamudpmulticast.h"

/**
 * @brief NewStonkamUdpMulticast::NewStonkamUdpMulticast
 */
NewStonkamUdpMulticast::NewStonkamUdpMulticast()
{
    mCameraListModel = nullptr;

    m_socket = new QUdpSocket(this);
    m_socket->bind(QHostAddress::AnyIPv4,
                   0,
                   QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    connect(m_socket, &QUdpSocket::readyRead,
            this, &NewStonkamUdpMulticast::processPendingDatagrams);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(sendReq()));
    m_timer->start(250);
}

/**
 * @brief NewStonkamUdpMulticast::~NewStonkamUdpMulticast
 */
NewStonkamUdpMulticast::~NewStonkamUdpMulticast()
{
    qDebug() << Q_FUNC_INFO;
    if (m_socket) {
        m_socket->close();
        m_socket->disconnect(this);
        //delete m_socket; //(parent = this)
        m_socket = nullptr;
    }
}

/**
 * @brief NewStonkamUdpMulticast::sendReq
 */
void NewStonkamUdpMulticast::sendReq()
{
    if(m_timer->interval() < 1000)
        m_timer->setInterval(1000);
    startDiscovery();
}

/**
 * @brief NewStonkamUdpMulticast::startDiscovery
 */
void NewStonkamUdpMulticast::startDiscovery()
{
    const QString probe = buildProbe();
    const QByteArray data = probe.toUtf8();

    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {

        if (!(iface.flags() & QNetworkInterface::IsUp) ||
            !(iface.flags() & QNetworkInterface::IsRunning) ||
            (iface.flags() & QNetworkInterface::IsLoopBack))
            continue;

        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {

            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            if (!mSelectedIp.isEmpty() && entry.ip().toString() != mSelectedIp)
                continue;

            qDebug() << "--> Send ONVIF probe     if" << iface.humanReadableName()
                     << " ip:" << entry.ip().toString();

            m_socket->setMulticastInterface(iface);

            for (int i = 0; i < NUMBER_OF_MULTI_REQ; ++i)
                m_socket->writeDatagram(data, MCAST_ADDR, MCAST_PORT);
        }
    }
}

void NewStonkamUdpMulticast::setSelectedInterface(const QString &ip)
{
    mSelectedIp = ip;
    qDebug() << "Multicast interface set to:" << (ip.isEmpty() ? "all" : ip);
}

/**
 * @brief NewStonkamUdpMulticast::buildProbe
 * @return
 */
QString NewStonkamUdpMulticast::buildProbe() const
{
    const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return QString(
               "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
               "<e:Envelope xmlns:e=\"http://www.w3.org/2003/05/soap-envelope\" "
               "xmlns:w=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
               "xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
               "xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\">"
               "<e:Header>"
               "<w:MessageID>uuid:%1</w:MessageID>"
               "<w:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</w:To>"
               "<w:Action>"
               "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"
               "</w:Action>"
               "</e:Header>"
               "<e:Body>"
               "<d:Probe>"
               "<d:Types>dn:NetworkVideoTransmitter</d:Types>"
               "</d:Probe>"
               "</e:Body>"
               "</e:Envelope>"
               ).arg(uuid);
}

/**
 * @brief NewStonkamUdpMulticast::processPendingDatagrams
 */
void NewStonkamUdpMulticast::processPendingDatagrams()
{
    while (m_socket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;
        m_socket->readDatagram(datagram.data(),
                               datagram.size(),
                               &sender,
                               &senderPort);

        QXmlStreamReader xml(datagram);
        while (!xml.atEnd())
        {
            xml.readNext();
            if (xml.isStartElement() &&
                xml.name().toString().contains("XAddrs"))
            {
                //const QString xaddr = xml.readElementText();
                //qDebug() << "  Endpoint ONVIF:" << xaddr << "\n";
                QString ip = sender.toString();
                QHostAddress netmask = getNetmaskForSender(sender);
                QString tmp = netmask.isNull() ? "" : netmask.toString();

                PanelItem tmpC;
                tmpC.hostname = "Stonkam";
                tmpC.machine  = "IPCamera";
                tmpC.foundEpoc= QDateTime::currentSecsSinceEpoch();
                tmpC.macaddr  = "";
                tmpC.ipv4addr = ip;
                tmpC.ipv4netmask = tmp;

                if(mCameraListModel && tmpC.ipv4addr != "")
                {
                    tmpC.macaddr = getMacForIP(tmpC.ipv4addr);
                    qDebug() << "<-- Recv ONVIF reply     from" << ip
                             << " MAC:" << (tmpC.macaddr.isEmpty() ? "n/a" : tmpC.macaddr)
                             << " netmask:" << (tmp.isEmpty() ? "n/a" : tmp);
                    if(tmpC.macaddr != "")
                        mCameraListModel->insertData(tmpC);
                }
            } //if (xml.isStartElement()
        } //while (!xml.atEnd())
    } //while (m_socket->hasPendingDatagrams())
}

/**
 * @brief NewStonkamUdpMulticast::getMacForIP
 * @param ipAddress
 * @return
 */
QString NewStonkamUdpMulticast::getMacForIP(QString ipAddress)
{
#if defined(Q_OS_LINUX)
    QFile arpTable("/proc/net/arp");
    if (!arpTable.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";

    QTextStream in(&arpTable);
    in.readLine(); // salta header
    while (!in.atEnd()) {
        QStringList fields = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        // colonne: IP, HW type, Flags, HW address, Mask, Device
        if (fields.size() >= 4 && fields.at(0) == ipAddress) {
            QString mac = fields.at(3);
            mac.remove(':');
            return mac;
        }
    }
    return "";

#elif defined(Q_OS_WIN)
    QString MAC = "";
    QProcess process;
    process.start("cmd.exe");
    process.write(QString("arp -a %1 \n\r").arg(ipAddress).toLocal8Bit());
    process.write("exit\n\r");

    if (process.waitForFinished()) {
        QString result = process.readAll();
        if (result.contains("No ARP Entries Found.", Qt::CaseInsensitive))
            return MAC;

        QStringList list = result.split(QRegularExpression("\\s+"));
        if (list.contains(ipAddress)) {
            int idx = list.indexOf(ipAddress);
            if (list.length() >= idx + 11)
                MAC = list.at(idx + 11);
        }
        MAC.remove('-');
    }
    return MAC;

#else
    Q_UNUSED(ipAddress)
    return "";
#endif
}

/**
 * @brief NewStonkamUdpMulticast::setCameraList
 * @param pl
 */
void NewStonkamUdpMulticast::setCameraList(PanelListModel *pl)
{
    mCameraListModel = pl;
}

/**
 * @brief NewStonkamUdpMulticast::netmaskForSender
 * @param sender
 * @return
 */
QHostAddress NewStonkamUdpMulticast::getNetmaskForSender(const QHostAddress &sender)
{
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {

        if (!(iface.flags() & QNetworkInterface::IsUp) ||
            !(iface.flags() & QNetworkInterface::IsRunning) ||
            (iface.flags() & QNetworkInterface::IsLoopBack))
            continue;

        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {

            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            if (entry.ip().isInSubnet(sender, entry.prefixLength())) {
                return entry.netmask();
            }
        }
    }

    return QHostAddress(); // invalida
}
