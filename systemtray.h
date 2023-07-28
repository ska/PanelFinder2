#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <QSystemTrayIcon>
#include <QObject>
#include <QAction>
#include <QMenu>

#include "common.h"

class SystemTray : public QObject
{
    Q_OBJECT
public:
    explicit SystemTray(QObject *parent = 0);

signals:
    void signalIconActivated();
    void signalShow();
    void signalQuit();
 
private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
 
public slots:
    void hideIconTray();
    Q_INVOKABLE void updateTooltip(quint16 p) const;
    Q_INVOKABLE void showHideMessage() const;

private:
    /* Declare the object of future applications for the tray icon*/
    QSystemTrayIcon         * trayIcon;

};
 
#endif // SYSTEMTRAY_H
