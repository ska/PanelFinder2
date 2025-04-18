#include "NewStonkamUdpMulticast.h"

/**
 * @brief NewStonkamUdpMulticast::NewStonkamUdpMulticast
 */
NewStonkamUdpMulticast::NewStonkamUdpMulticast()
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    foreach(QNetworkInterface interface, interfaces)
    {

        if ( interface.flags().testFlag(QNetworkInterface::IsUp) &&
             interface.flags().testFlag(QNetworkInterface::IsRunning) &&
             interface.flags().testFlag(QNetworkInterface::CanMulticast) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack) &&
#ifdef Q_OS_LINUX
              (interface.name().contains("eth", Qt::CaseInsensitive) || interface.name().contains("ens", Qt::CaseInsensitive))
#else
               interface.humanReadableName().contains("Ethernet", Qt::CaseInsensitive)
#endif
            )
        {
            /*
             * If here Interface is:
             * - IsUp
             * - IsRunning
             * - CanMulticast
             * - NOT IsLoopBack
             * - isCopper
             */
            qDebug() << "Interface: " << interface.humanReadableName();

            listMC.append(new QUdpSocket());
            listMC.last()->setSocketOption(QAbstractSocket::LowDelayOption, 1);

            QList<QNetworkAddressEntry> allEntries = interface.addressEntries();
            QHostAddress toBind; //("192.168.88.20");
            foreach (QNetworkAddressEntry entry, allEntries)
            {
                qDebug() << "  --> " << entry.ip().toString() << "/" << entry.netmask().toString();
                toBind = entry.ip();

                /* Ip camera sembra accettare le richieste solo se arrivano da porta 2887 dirette a 2887 */
                if(listMC.last()->bind(toBind, 2887, QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint))
                {
                    QList<QNetworkAddressEntry> addressEntries = interface.addressEntries();
                    for (int i = 0; i < addressEntries.length(); i++)
                    {
                        QNetworkAddressEntry ae = addressEntries.at(i);
                        if(ae.ip() == toBind)
                        {
                            bool ok = false;
                            if (ae.ip().protocol() == QAbstractSocket::IPv4Protocol)
                            {
                                ok = listMC.last()->joinMulticastGroup(QHostAddress("239.255.255.255"), interface);
                            }
                            if(ok)
                            {
                                qDebug()<<"  SA bound... join mc group:" << ae.ip();
                                connect(listMC.last(), &QUdpSocket::readyRead, this, &NewStonkamUdpMulticast::readPendingDatagrams);
                            }
                            else
                            {
                                qDebug()<<"  SA bound... interface unsuitable for Multicast:"<<ae.ip();
                            }
                        }
                    }
                } //if(saMC->bind
            } //foreach (entry, allEntries)
        } //if flags
    } //foreach(QNetworkInterface interface, interfaces)

    mCameraListModel = nullptr;

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
    for( auto i = 0; i<listMC.length(); i++)
    {
        delete( listMC.at(i) );
    }
    listMC.clear();
}

/**
 * @brief NewStonkamUdpMulticast::sendReq
 */
void NewStonkamUdpMulticast::sendReq()
{
    if(m_timer->interval() < 1000)
        m_timer->setInterval(1000);

    for( auto i = 0; i<listMC.length(); i++)
    {
        listMC.at(i)->writeDatagram(datagramReq, mcAddr, 2887);
    }
}

/**
 * @brief NewStonkamUdpMulticast::readPendingDatagrams
 */
void NewStonkamUdpMulticast::readPendingDatagrams()
{
    PanelItem tmpC;
    for( auto i = 0; i<listMC.length(); i++)
    {
        while (listMC.at(i)->hasPendingDatagrams())
        {
            QNetworkDatagram datagram = listMC.at(i)->receiveDatagram();

            if(!QString(datagram.data()).contains("IPAddress", Qt::CaseSensitive))
                continue;

            /*XML non standard, aggiungo qualche pezzo per renderlo std e poter usare QXmlStreamReader*/
            QString dataStr = (QString(datagram.data()));
            dataStr.replace(QString("?><IP"), QString("?><IPCamera><IP"));
            dataStr.append("</IPCamera>");

            QXmlStreamReader xmlResponse;
            xmlResponse.addData(dataStr.toUtf8());

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
                        QString tmp = xmlResponse.readElementText();
                        //qDebug() << "Camera IPAddress: " << tmp;
                        tmpC.ipv4addr = tmp;
                    }

                    if(xmlResponse.name().toString() == "Gateway")
                    {
                        QString tmp = xmlResponse.readElementText();
                        //qDebug() << "Camera Gateway: " << tmp;
                    }

                    if(xmlResponse.name().toString() == "Submask")
                    {
                        QString tmp = xmlResponse.readElementText();
                        //qDebug() << "Camera Submask: " << tmp;
                        tmpC.ipv4netmask = tmp;

                    }

                }
            }

            if(mCameraListModel && tmpC.ipv4addr != "")
            {
                tmpC.macaddr = getMacForIP( tmpC.ipv4addr );
                if(tmpC.macaddr != "")
                    mCameraListModel->insertData(tmpC);
            }
        }
    }

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
