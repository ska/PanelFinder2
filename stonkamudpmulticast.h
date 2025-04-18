#ifndef STONKAMUDPMULTICAST_H
#define STONKAMUDPMULTICAST_H

#include <QTimer>
#include <QObject>
#include <QtNetwork>
#include <QDateTime>
#include "panellistmodel.h"

class StonkamUdpMulticast : public QObject
{
    Q_OBJECT
public:
    StonkamUdpMulticast();
    ~StonkamUdpMulticast();

    void startSearchTimer();
    void setPanelList(PanelListModel *pl);
    static QString getMacForIP(QString ipAddress);

    Q_INVOKABLE void testString(QString string);

private slots:
    void startSearch();
    void processPendingDatagrams();

private:
    QTimer *mtimer;
    QUdpSocket *m_udpSocket4;
    QHostAddress m_groupAddress4;
    QStringList m_foundIPCamerasStr;
    PanelListModel *mPanelListModel;
    void setMulticast();
};

#endif // STONKAMUDPMULTICAST_H
