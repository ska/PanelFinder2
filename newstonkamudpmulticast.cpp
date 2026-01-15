#include "NewStonkamUdpMulticast.h"

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

    //m_timeoutTimer->start();

    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {

        if (!(iface.flags() & QNetworkInterface::IsUp) ||
            !(iface.flags() & QNetworkInterface::IsRunning) ||
            (iface.flags() & QNetworkInterface::IsLoopBack))
            continue;

        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {

            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            qDebug() << "--> Send onvif probe on if"
                        << iface.humanReadableName()
                        << entry.ip().toString();

            m_socket->setMulticastInterface(iface);

            for (int i = 0; i < NUMBER_OF_MULTI_REQ; ++i)
            {
                m_socket->writeDatagram(
                    data,
                    MCAST_ADDR,
                    MCAST_PORT);
            }
        }
    }
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
                QString tmp = "";
                if (!netmask.isNull()) {
                    qDebug() << "Camera:" << ip
                             << "Netmask:" << netmask.toString();
                    tmp = netmask.toString();
                } else {
                    qDebug() << "Camera:" << ip
                             << "Netmask: non trovata";
                }

                PanelItem tmpC;
                tmpC.hostname = "Stonkam";
                tmpC.machine  = "IPCamera";
                tmpC.foundEpoc= QDateTime::currentSecsSinceEpoch();
                tmpC.macaddr  = "";
                tmpC.ipv4addr = ip;
                tmpC.ipv4netmask = tmp;

                if(mCameraListModel && tmpC.ipv4addr != "")
                {
                    tmpC.macaddr = getMacForIP( tmpC.ipv4addr );
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
    QString MAC = "";
    QRegularExpression regex = QRegularExpression("\\s+");
    QProcess process;
    process.start("cmd.exe");
    process.write( QString("arp -a %1 \n\r").arg(ipAddress).toLocal8Bit());
    process.write ("exit\n\r");

    if(process.waitForFinished())
    {
        QString result = process.readAll();
        if(result.contains("No ARP Entries Found.", Qt::CaseInsensitive ))
        {
            return MAC;
        }

        QStringList list = result.split(regex);
        if(list.contains(ipAddress))
        {
            if(list.length() >= list.indexOf(ipAddress) + 11)
                MAC = list.at(list.indexOf(ipAddress) + 11);
        }
        MAC = MAC.replace('-', "");
    }

    return MAC;
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
