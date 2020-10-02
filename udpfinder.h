#ifndef UDPFINDER_H
#define UDPFINDER_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QStringListModel>
#include "panellistmodel.h"

typedef struct
{
    char identification[10];
    char opcode[2];
    char hardware[16];
    char ip[16];
    char netmask[16];
    char RemoteConfiguratorPort[4];
    char moduleName[5];
    char manufactureCode[2];
    char reserved[12];
    char hostName[64];
} struct_selfinfo1;

class UdpFinder : public QObject
{
    Q_OBJECT
    //Q_PROPERTY(QStringListModel model MEMBER mIpaddrModel NOTIFY modelChanged)

public:
    explicit UdpFinder(QObject *parent = nullptr);
    void setPanelList(PanelListModel *pl);
    Q_INVOKABLE void testString(QString string);


    QStringList ipaddr() const;

signals:
    void modelChanged(void);

public slots:
    void scanCmd();
    void readyRead();

private:
    QUdpSocket *msocket;
    PanelListModel *mPanelListModel;
    QStringList mIpaddr;
    QTimer *mtimer;
    quint8 mcase;
    quint8 m_SelectedInterface;
    quint8 m_RunningInterface;

};

#endif // UDPFINDER_H
