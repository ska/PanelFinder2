#include "systemtray.h"

SystemTray::SystemTray(QObject *parent) : QObject(parent)
{
    QMenu *trayIconMenu = new QMenu();
    QAction * viewWindow = new QAction(("Show"), this);
    QAction * quitAction = new QAction(("Quit"), this);

    trayIconMenu->addAction(viewWindow);
    trayIconMenu->addAction(quitAction);

    connect(viewWindow, &QAction::triggered, this, &SystemTray::signalShow);
    connect(quitAction, &QAction::triggered, this, &SystemTray::signalQuit);

    trayIcon = new QSystemTrayIcon();
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/pics/icon.ico"));
    trayIcon->show();
    trayIcon->setToolTip(SW_NAME);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

}
 
/* The method that handles click on the application icon in the system tray
 * */
void SystemTray::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason){
    case QSystemTrayIcon::Trigger:
        // In the case of pressing the signal on the icon tray in the call signal QML layer
        emit signalIconActivated();
        break;
    default:
        break;
    }
}
 
void SystemTray::hideIconTray()
{
    trayIcon->hide();
}

void SystemTray::updateTooltip(quint16 p) const
{
    trayIcon->setToolTip(QString("PanelFinder2 \nFound %1 panels").arg(p) );
}

void SystemTray::showHideMessage() const
{
    QString message = "The program will keep running in the "
                      "system tray. To terminate the program, "
                      "choose Quit in the context menu "
                      "of the system tray entry.";
    trayIcon->showMessage(SW_NAME, message, QIcon(":/pics/icon.ico"), 1 * 100);
}
