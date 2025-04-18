#include <QQmlApplicationEngine>
#include <QApplication>
#include <QQmlContext>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>

#include "newstonkamudpmulticast.h"
#include "panellistmodel.h"
#include "systemtray.h"
#include "udpfinder.h"
#include "runguard.h"
#include "common.h"
#include "serviceudp.h"


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(SW_NAME);
    QCoreApplication::setApplicationVersion(SW_VER);
    QFileInfo fi(argv[0]);

    qInfo().noquote() << fi.fileName() << " SW Build name: "    << SW_NAME;
    qInfo().noquote() << fi.fileName() << " Build version: "    << SW_VER;

    app.setWindowIcon(QIcon(":/pics/icon.ico"));
    app.setQuitOnLastWindowClosed(false);

#ifdef SERVICEUDP_H
    ServiceUDP serviceUDP;
#endif

#ifdef RUNGUARD_H
    /* One instance only
     * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    RunGuard guard( "76c23839795022fc167bde920c208549" );
    if ( !guard.tryToRun() )
    {
#ifdef SERVICEUDP_H
        serviceUDP.sayAlreadyRunning();
#else
        QMessageBox msgBox;
        msgBox.setText("Application already running.\nCheck trayicon.");
        msgBox.exec();
#endif

        qCritical("Application already running");
        return -1;
    }
#endif

    QClipboard *clipboard = QGuiApplication::clipboard();

    PanelListModel listModel;

    //Create filter model
    FilterProxyModel filterModel;
    filterModel.setSourceModel(&listModel);
    filterModel.setFilterRole(MacaddressRole);
    filterModel.setSortRole(MacaddressRole);
    filterModel.setClipboard(clipboard);

    //Create udp obj
    UdpFinder *udpfinder = new UdpFinder();
    udpfinder->setPanelList(&listModel);

    //Create udp multicast obj
    NewStonkamUdpMulticast *sk = new NewStonkamUdpMulticast();
    sk->setCameraList(&listModel);


    QQmlApplicationEngine engine;
    QQmlContext* context = engine.rootContext();
    context->setContextProperty("filterModelQml", &filterModel);
    context->setContextProperty("listModelQml", &listModel);

    QStringListModel netIfModel;
    netIfModel.setStringList(udpfinder->ipaddr());
    context->setContextProperty("udpfinderModel", &netIfModel);
    context->setContextProperty("udpfinderQml", udpfinder);

    SystemTray * systemTray = new SystemTray();
    context->setContextProperty("systemTray", systemTray);
    context->setContextProperty("serviceUDP", &serviceUDP);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    int ret = app.exec();
    qInfo() << fi.fileName() << " Closing!!!";
    systemTray->hideIconTray();

    return ret;
}
