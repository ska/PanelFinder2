#include "udpfinder.h"
#include <QThread>
#include <QNetworkInterface>

UdpFinder::UdpFinder(QObject *parent)
    : QObject(parent)
    , msocket(nullptr)
{
    mcase = 0;
    m_SelectedInterface = 0;
    mIpaddr.clear();
    mIpaddr.append( "All interfaces" );

    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface iface, list)
    {
        if(  iface.flags().testFlag(QNetworkInterface::IsUp) && \
             iface.flags().testFlag(QNetworkInterface::IsRunning) && \
             !iface.flags().testFlag(QNetworkInterface::IsLoopBack) && \
             !iface.flags().testFlag(QNetworkInterface::IsPointToPoint)
             )
        {
            foreach (QNetworkAddressEntry entry,  iface.addressEntries() )
            {
                if( entry.ip().protocol() == QAbstractSocket::IPv4Protocol )
                {
                    int index = iface.index();
                    qDebug() << QString("ETH Interface: %1").arg(index) +\
                                "  "+ iface.name() +\
                                "  "+ entry.ip().toString() +\
                                "  "+ iface.hardwareAddress();
                    mIpaddr.append( entry.ip().toString() );
                }
            }
        }
    }
    mPanelListModel = nullptr;
    //this->scanCmd();
    mtimer = new QTimer(this);
    connect(mtimer, SIGNAL(timeout()), this, SLOT(scanCmd()));
    mtimer->start(100);

}

void UdpFinder::setPanelList(PanelListModel *pl)
{
    mPanelListModel = pl;
}

void UdpFinder::scanCmd()
{
    if(mtimer->interval() < 1000)
        mtimer->setInterval(1000);

    if(msocket && !msocket->hasPendingDatagrams())
    {
        msocket->close();
        msocket = nullptr;
    }

    if(!msocket)
    {
        msocket = new QUdpSocket(this);
        if(m_SelectedInterface == 0 && m_RunningInterface == mIpaddr.size())
        {
            m_RunningInterface = 0;
        }
        if(m_SelectedInterface != 0)
        {
            m_RunningInterface = m_SelectedInterface;
        }

        msocket->bind(QHostAddress( mIpaddr.at(m_RunningInterface)), 9999);
        connect(msocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    }
    QByteArray Data;
    Data.clear();

    //qDebug() << "m_SelectedInterface: " << m_SelectedInterface;
    //qDebug() << "mcase: " << mcase << "m_RunningInterface: " << m_RunningInterface;
    qint64 ret = 0;

#ifdef TEST
    switch(mcase)
    {
        case 0:
            Data.append("WhereAreYou.02!");
            ret = msocket->writeDatagram(Data, Data.size(), QHostAddress::Broadcast, 991);
            mcase++;
            break;

        case 1:
            Data.append("WhereAreYou.01!");
            ret = msocket->writeDatagram(Data, Data.size(), QHostAddress::Broadcast, 991);
            mcase++;
            break;

        case 2:
            Data.append("WhereAreYou.02");
            ret = msocket->writeDatagram(Data, Data.size(), QHostAddress::Broadcast, 991);
            mcase++;

        default:
            mcase = 0;
            if(m_SelectedInterface == 0)
            {
                m_RunningInterface ++;
            }
            break;
    }
#else
    Data.append("WhereAreYou.02");
    msocket->writeDatagram(Data,QHostAddress::Broadcast,991);
    if(m_SelectedInterface == 0)
    {
        m_RunningInterface ++;
    }
#endif

}

void UdpFinder::readyRead()
{
    QByteArray buffer;
    buffer.resize(msocket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;

    quint16 bytes_received = msocket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort );
    if (strncmp("I am here.0", buffer, 11) == 0 && ( buffer[11] == '2' || buffer[11] == '3') )
    {
        struct_selfinfo1 tmp2;
        memset(&tmp2, 0, sizeof(struct_selfinfo1));
        memcpy(&tmp2, buffer, sizeof(struct_selfinfo1));
        QString hostName = "";
        QString mac =     QString(QByteArray::fromRawData(tmp2.hardware,   sizeof(tmp2.hardware))).trimmed().toLatin1();
        //QString ip =      QString(QByteArray::fromRawData(tmp2.ip,         sizeof(tmp2.ip))).trimmed().toLatin1();
        QString ip =      sender.toString().trimmed().toLatin1();
        QString netmask = QString(QByteArray::fromRawData(tmp2.netmask,    sizeof(tmp2.netmask))).trimmed().toLatin1();
        QString moduleName = QString(QByteArray::fromRawData(tmp2.moduleName, sizeof(tmp2.moduleName))).trimmed().toLatin1();
        if( 148 == bytes_received )
            hostName = QString(QByteArray::fromRawData(tmp2.hostName, sizeof(tmp2.hostName))).trimmed().toLatin1();

        if(mPanelListModel)
            mPanelListModel->insertData({hostName, moduleName, mac, ip, netmask, QDateTime::currentSecsSinceEpoch()});
    }
}

QStringList UdpFinder::ipaddr() const
{
    return mIpaddr;
}

void UdpFinder::testString(QString string)
{
    m_SelectedInterface = string.toUInt();
    qDebug() << "Selected interface NUM: " << QString("%1").arg((quint16)m_SelectedInterface, 0, 10).toUpper();
    mcase = 0;
    m_RunningInterface = 0;
    //mPanelListModel->clearList();
    mtimer->setInterval(1000);
}
