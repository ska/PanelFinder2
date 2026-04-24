#include "udpfinder.h"
#include <QThread>
#include <QNetworkInterface>

UdpFinder::UdpFinder(QObject *parent)
    : QObject(parent)
    , msocket(nullptr)
{
    mcase = 0;
    m_SelectedInterface = 0;
    m_RunningInterface  = 0;
    mIpaddr.append("All interfaces");
    mBroadcastAddr.append("");

    refreshInterfaces();

    mPanelListModel = nullptr;
    mtimer = new QTimer(this);
    connect(mtimer, SIGNAL(timeout()), this, SLOT(scanCmd()));
    mtimer->start(100);
}

void UdpFinder::refreshInterfaces()
{
    QStringList newIpaddr;
    QStringList newBcast;
    newIpaddr.append("All interfaces");
    newBcast.append("");

    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces())
    {
        if (!iface.flags().testFlag(QNetworkInterface::IsUp)      ||
            !iface.flags().testFlag(QNetworkInterface::IsRunning) ||
             iface.flags().testFlag(QNetworkInterface::IsLoopBack)||
             iface.flags().testFlag(QNetworkInterface::IsPointToPoint))
            continue;

        for (const QNetworkAddressEntry &entry : iface.addressEntries())
        {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            QString bcast = entry.broadcast().isNull() ? "255.255.255.255"
                                                       : entry.broadcast().toString();
            newIpaddr.append(entry.ip().toString());
            newBcast.append(bcast);
        }
    }

    if (newIpaddr == mIpaddr)
        return;

    // If the selected interface disappeared, fall back to "All interfaces"
    QString selectedIp = (m_SelectedInterface > 0 && m_SelectedInterface < (quint8)mIpaddr.size())
                         ? mIpaddr.at(m_SelectedInterface) : "";

    mIpaddr        = newIpaddr;
    mBroadcastAddr = newBcast;

    quint8 prevSelected = m_SelectedInterface;
    if (!selectedIp.isEmpty())
    {
        int idx = mIpaddr.indexOf(selectedIp);
        m_SelectedInterface = (idx >= 0) ? static_cast<quint8>(idx) : 0;
    }
    m_RunningInterface = 0;

    qDebug() << "Network interfaces changed:" << mIpaddr;
    emit interfacesChanged(mIpaddr);

    if (m_SelectedInterface != prevSelected)
    {
        emit interfaceSelected(m_SelectedInterface > 0 ? mIpaddr.at(m_SelectedInterface) : "");
        emit currentInterfaceIndexChanged();
    }
}

void UdpFinder::setPanelList(PanelListModel *pl)
{
    mPanelListModel = pl;
}

void UdpFinder::scanCmd()
{
    if(mtimer->interval() < 1000)
        mtimer->setInterval(1000);

    refreshInterfaces();

    if(msocket && !msocket->hasPendingDatagrams())
    {
        disconnect(msocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        msocket->close();
        delete msocket;
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

        QHostAddress bindAddr = (m_RunningInterface == 0)
                                ? QHostAddress::AnyIPv4
                                : QHostAddress(mIpaddr.at(m_RunningInterface));
        msocket->bind(bindAddr, 9999);
        connect(msocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    }
    QByteArray Data;
    Data.clear();

    Data.append("WhereAreYou.02");
    QHostAddress broadcastAddr = (m_RunningInterface == 0 || m_RunningInterface >= mBroadcastAddr.size())
                                 ? QHostAddress::Broadcast
                                 : QHostAddress(mBroadcastAddr.at(m_RunningInterface));
    msocket->writeDatagram(Data, broadcastAddr, 991);
    qDebug() << "--> Send UDP discovery  if"
             << (m_RunningInterface == 0 ? QString("All") : mIpaddr.at(m_RunningInterface))
             << " bcast:" << broadcastAddr.toString();
    if(m_SelectedInterface == 0)
    {
        m_RunningInterface ++;
    }
}

void UdpFinder::readyRead()
{
    QByteArray buffer;
    buffer.resize(msocket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;

    quint16 bytes_received = msocket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
    qDebug() << "<-- Recv UDP discovery   from" << sender.toString() << bytes_received << "bytes";
    if (bytes_received < sizeof(struct_selfinfo1))
        return;
    if (strncmp("I am here.0", buffer, 11) == 0 && ( buffer[11] == '2' || buffer[11] == '3') )
    {
        struct_selfinfo1 tmp2;
        memset(&tmp2, 0, sizeof(struct_selfinfo1));
        memcpy(&tmp2, buffer, sizeof(struct_selfinfo1));
        QString hostName = "";
        QString mac        = QString(QByteArray::fromRawData(tmp2.hardware,   sizeof(tmp2.hardware))).trimmed().toLatin1();
        QString ip         = sender.toString().trimmed().toLatin1();
        QString netmask    = QString(QByteArray::fromRawData(tmp2.netmask,    sizeof(tmp2.netmask))).trimmed().toLatin1();
        QString moduleName = QString(QByteArray::fromRawData(tmp2.moduleName, sizeof(tmp2.moduleName))).trimmed().toLatin1();
        if (148 == bytes_received)
            hostName = QString(QByteArray::fromRawData(tmp2.hostName, sizeof(tmp2.hostName))).trimmed().toLatin1();

        qDebug() << "    Panel:" << ip << "MAC:" << mac << "module:" << moduleName << "host:" << hostName;
        if(mPanelListModel)
            mPanelListModel->insertData({hostName, moduleName, mac, ip, netmask, "", "", "", QDateTime::currentSecsSinceEpoch()});
    }

}

QStringList UdpFinder::ipaddr() const
{
    return mIpaddr;
}

void UdpFinder::testString(QString string)
{
    uint val = string.toUInt();
    quint8 prev = m_SelectedInterface;
    m_SelectedInterface = (val <= (uint)(mIpaddr.size() - 1)) ? static_cast<quint8>(val) : 0;
    qDebug() << "Selected interface NUM:" << m_SelectedInterface;
    mcase = 0;
    m_RunningInterface = 0;
    if (mPanelListModel)
        mPanelListModel->clearList();
    mtimer->setInterval(100);

    QString selectedIp = (m_SelectedInterface > 0) ? mIpaddr.at(m_SelectedInterface) : "";
    emit interfaceSelected(selectedIp);
    if (m_SelectedInterface != prev)
        emit currentInterfaceIndexChanged();
}
