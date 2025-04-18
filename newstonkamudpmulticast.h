#ifndef NEWSTONKAMUDPMULTICAST_H
#define NEWSTONKAMUDPMULTICAST_H

#include <QtNetwork>
#include <QtCore>
#include <QObject>
#include <QList>
#include "panellistmodel.h"

class NewStonkamUdpMulticast: public QObject
{
    Q_OBJECT

public:
    NewStonkamUdpMulticast();
    ~NewStonkamUdpMulticast();
    void setCameraList(PanelListModel *pl);
    static QString getMacForIP(QString ipAddress);

private slots:
    void readPendingDatagrams();
    void sendReq();


private:
    QList <QUdpSocket*> listMC;
    QTimer              *m_timer;
    PanelListModel      *mCameraListModel;

    const QHostAddress  mcAddr = QHostAddress("239.255.255.255");
    const QByteArray    datagramReq = "<?xml version=\"1.0\" encoding=\"utf-8\"?><Types>stonkam ipsearch</Types>";
};

#endif // NEWSTONKAMUDPMULTICAST_H
