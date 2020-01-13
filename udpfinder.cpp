#include "udpfinder.h"
#include <QThread>
#include <QNetworkInterface>

UdpFinder::UdpFinder(QObject *parent)
    : QObject(parent)
    , msocket(nullptr)
{
    mtest = 0;
    mipaddr.clear();

    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
        {
            mipaddr.append( address.toString() );
        }
    }
    mPanelListModel = nullptr;
    this->scanCmd();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(scanCmd()));
    timer->start(2000);

}

void UdpFinder::setPanelList(PanelListModel *pl)
{
    mPanelListModel = pl;
}

void UdpFinder::scanCmd()
{
    if(msocket && !msocket->hasPendingDatagrams())
    {
        msocket->close();
        msocket = nullptr;
    }

    if(!msocket)
    {
        msocket = new QUdpSocket(this);
        mtest++;
        if(mtest == mipaddr.size())
            mtest = 0;

        msocket->bind(QHostAddress( mipaddr.at(mtest)), 9999);
        connect(msocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    }
    QByteArray Data;
    Data.clear();
    Data.append("WhereAreYou.02");
    msocket->writeDatagram(Data,QHostAddress::Broadcast,991);

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
