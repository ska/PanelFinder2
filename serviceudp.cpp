#include "serviceudp.h"

ServiceUDP::ServiceUDP(QObject *parent) :
    QObject(parent)
{
    socket = new QUdpSocket(this);

    //We need to bind the UDP socket to an address and a port
    socket->bind(QHostAddress::LocalHost,61234);         //ex. Address localhost, port 1234

    connect(socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
}

void ServiceUDP::sayAlreadyRunning()      //Just spit out some data
{
    QByteArray Data;
    Data.append(0x01);
    Data.append("    ");
    Data.append(SW_NAME);
    Data.append(" - ");
    Data.append(SW_VER);
    Data.append(" - Application already sunning");
    socket->writeDatagram(Data,QHostAddress::LocalHost,61234);
}


void ServiceUDP::readyRead()     //Read something
{
    QByteArray Buffer;
    Buffer.resize(socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;
    socket->readDatagram(Buffer.data(),Buffer.size(),&sender,&senderPort);

    //qDebug() << "Buffer: " << Buffer;

    switch(Buffer.at(0))
    {
    case 0x01:
        //qDebug() << "0x01 Running";
        emit guiMaximize();
        break;
    default:
        qWarning() << "Boh";
        break;
    }


}
