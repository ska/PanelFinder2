#include "stonkamudpmulticast.h"

QString StonkamUdpMulticast::getMacForIP(QString ipAddress)
{
    QString MAC = "";
    QProcess process;
    process.start("cmd.exe");
    process.write( QString("arp -a %1 \n\r").arg(ipAddress).toLocal8Bit());
    process.write ("exit\n\r");

    if(process.waitForFinished())
    {
        QString result = process.readAll();
        QStringList list = result.split(QRegularExpression("\\s+"));
        if(list.contains(ipAddress))
            MAC = list.at(list.indexOf(ipAddress) + 11);
        MAC = MAC.replace('-', ':');
    }

    return MAC;
}

StonkamUdpMulticast::StonkamUdpMulticast()
{
    bool ret;
    //qDebug() << Q_FUNC_INFO << "Enter";
    m_groupAddress4 = QHostAddress("239.255.255.255");
    m_udpSocket4 = new QUdpSocket();
    m_udpSocket4->setProxy(QNetworkProxy::NoProxy);

    ret = m_udpSocket4->bind( QHostAddress::AnyIPv4, 2887, QUdpSocket::ShareAddress);
    if(!ret)
        qWarning() << "bind err: " << m_udpSocket4->errorString();

    m_udpSocket4->setSocketOption(QAbstractSocket::MulticastTtlOption, 255);

    //ret = m_udpSocket4->joinMulticastGroup(m_groupAddress4);
    QList<QNetworkInterface> mListIfaces = QNetworkInterface::allInterfaces();

    for (int i = 0; i < mListIfaces.length(); ++i) {
        bool rez = m_udpSocket4->joinMulticastGroup(m_groupAddress4, mListIfaces.at(i));
        //qDebug() << rez;
    }


    if(!ret)
        qWarning() << "bind err: " << m_udpSocket4->errorString();

    connect(m_udpSocket4, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));

}

StonkamUdpMulticast::~StonkamUdpMulticast()
{
    //qDebug() << Q_FUNC_INFO;
    bool ret = false;
    ret = m_udpSocket4->leaveMulticastGroup(m_groupAddress4);

    QList<QNetworkInterface> mListIfaces = QNetworkInterface::allInterfaces();
    for (int i = 0; i < mListIfaces.length(); ++i) {
        bool rez = m_udpSocket4->leaveMulticastGroup(m_groupAddress4, mListIfaces.at(i));
        //qDebug() << rez;
    }

    qInfo() << "m_udpSocket4->leaveMulticastGroup: " << ret;
    disconnect(m_udpSocket4, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
    m_udpSocket4->abort();
    delete m_udpSocket4;
    m_udpSocket4 = nullptr;

    //qDebug() << Q_FUNC_INFO << " END";
}

void StonkamUdpMulticast::setPanelList(PanelListModel *pl)
{
    mPanelListModel = pl;
}


void StonkamUdpMulticast::startSearch()
{
    //qDebug() << Q_FUNC_INFO;
    if(mtimer->interval() < 1000)
        mtimer->setInterval(5000);

    if (!m_udpSocket4)
        return;

    QByteArray datagram = "<?xml version=\"1.0\" encoding=\"utf-8\"?><Types>stonkam ipsearch</Types>";
    m_udpSocket4->writeDatagram(datagram, m_groupAddress4, 2887);
}

void StonkamUdpMulticast::startSearchTimer()
{
    //qDebug() << Q_FUNC_INFO;
    mtimer = new QTimer(this);
    connect(mtimer, SIGNAL(timeout()), this, SLOT(startSearch()));
    mtimer->start(100);

}

void StonkamUdpMulticast::processPendingDatagrams()
{
    //qDebug() << Q_FUNC_INFO;
    QHostAddress sender;
    quint16 senderPort;
    PanelItem tmpC;

    while (m_udpSocket4->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(m_udpSocket4->pendingDatagramSize());
        m_udpSocket4->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        /*XML non standard, aggiungo qualche pezzo per renderlo std e poter usare QXmlStreamReader*/
        QString dataStr = (QString(datagram.data()));
        dataStr.replace(QString("?><IP"), QString("?><IPCamera><IP"));
        dataStr.append("</IPCamera>");

        datagram.clear();
        datagram = dataStr.toUtf8();

        QXmlStreamReader xmlResponse;
        xmlResponse.addData(datagram);
        m_foundIPCamerasStr.clear();

        while(!xmlResponse.atEnd() && !xmlResponse.hasError())
        {
            QXmlStreamReader::TokenType token = xmlResponse.readNext();
            //If token is just StartDocument - go to next
            if(token == QXmlStreamReader::StartDocument) {
                continue;
            }
            //If token is StartElement - read it
            if(token == QXmlStreamReader::StartElement)
            {

                tmpC.hostname = "Stonkam";
                tmpC.machine  = "IPCamera";
                tmpC.foundEpoc= QDateTime::currentSecsSinceEpoch();
                tmpC.macaddr  = "";
                if(xmlResponse.name().toString() == "IPAddress")
                {
                    //qDebug() << "IPAddress: " << xmlResponse.readElementText();
                    tmpC.ipv4addr = xmlResponse.readElementText();
                }

                if(xmlResponse.name().toString() == "Gateway")
                {
                }

                if(xmlResponse.name().toString() == "Submask")
                {
                    //qDebug() << "Submask: " << xmlResponse.readElementText();
                    tmpC.ipv4netmask = xmlResponse.readElementText();
                }
            }
        }

        if(mPanelListModel && tmpC.ipv4addr != "")
        {
            tmpC.macaddr = getMacForIP( tmpC.ipv4addr );
            mPanelListModel->insertData(tmpC);
        }
    }
}
