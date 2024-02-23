#ifndef SERVICEUDP_H
#define SERVICEUDP_H

#include <QUdpSocket>
#include "common.h"

class ServiceUDP : public QObject
{
    Q_OBJECT

public:
    explicit ServiceUDP(QObject *parent = 0);

    void sayAlreadyRunning();

signals:
    void guiMaximize();

public slots:
    void readyRead();

private:
    QUdpSocket *socket;
};

#endif // SERVICEUDP_H
