#ifndef NEWSTONKAMUDPMULTICAST_H
#define NEWSTONKAMUDPMULTICAST_H

#include <QtNetwork>
#include <QtCore>
#include <QObject>
#include <QList>
#include "panellistmodel.h"

inline const QHostAddress   MCAST_ADDR("239.255.255.250");
inline constexpr quint16    MCAST_PORT = 3702;
inline constexpr quint16    NUMBER_OF_MULTI_REQ = 1;
inline constexpr quint16    DISCOVERY_TIMEOUT = 500;

class NewStonkamUdpMulticast: public QObject
{
    Q_OBJECT

public:
    NewStonkamUdpMulticast();
    ~NewStonkamUdpMulticast();
    void setCameraList(PanelListModel *pl);
    void startDiscovery();
    static QString getMacForIP(QString ipAddress);
    static QHostAddress getNetmaskForSender(const QHostAddress &sender);

private slots:
    void sendReq();
    void processPendingDatagrams();


private:
    QTimer              *m_timer;
    PanelListModel      *mCameraListModel;
    QUdpSocket          *m_socket;

    QString buildProbe() const;

};

#endif // NEWSTONKAMUDPMULTICAST_H
