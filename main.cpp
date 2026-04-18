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
#include "mysplashscreen.h"

#ifdef MYSPLASHSCREEN_H
#define MY_SPLASH_SET_PROGRESS(splash, x) splash.setProgress(x)
#define MY_SPLASH_HIDE(splash)           splash.hide()
#else
#define MY_SPLASH_SET_PROGRESS(splash, x)  ((void)0)
#define MY_SPLASH_HIDE(splash)            ((void)0)
#endif

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(SW_NAME);
    QCoreApplication::setApplicationVersion(SW_VER);


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


#ifdef MYSPLASHSCREEN_H
    QScreen *activeScreen = QGuiApplication::screenAt(QCursor::pos());
    if (!activeScreen) {
        activeScreen = QGuiApplication::primaryScreen();
    }

    MySplashScreen splash(QPixmap(":/pics/splash_400.png"));
    splash.move(
        activeScreen->geometry().center() - splash.rect().center()
    );
    splash.show();
    app.processEvents();
    splash.setStatusText(SW_NAME " " SW_VER);
#endif
    MY_SPLASH_SET_PROGRESS(splash, 10);


    QFileInfo fi(argv[0]);
    qInfo().noquote() << fi.fileName() << " SW Build name: "    << SW_NAME;
    qInfo().noquote() << fi.fileName() << " Build version: "    << SW_VER;

    app.setWindowIcon(QIcon(":/pics/icon.ico"));
    app.setQuitOnLastWindowClosed(false);

    MY_SPLASH_SET_PROGRESS(splash, 40);
    QClipboard *clipboard = QGuiApplication::clipboard();

    PanelListModel listModel;

    //Create filter model
    MY_SPLASH_SET_PROGRESS(splash, 50);
    FilterProxyModel filterModel;
    filterModel.setSourceModel(&listModel);
    filterModel.setFilterRole(MacaddressRole);
    filterModel.setSortRole(MacaddressRole);
    filterModel.setClipboard(clipboard);

    //Create udp obj
    MY_SPLASH_SET_PROGRESS(splash, 60);
    UdpFinder *udpfinder = new UdpFinder();
    udpfinder->setPanelList(&listModel);

    //Create udp multicast obj
    MY_SPLASH_SET_PROGRESS(splash, 70);
    NewStonkamUdpMulticast *sk = new NewStonkamUdpMulticast();
    sk->setCameraList(&listModel);

    QQmlApplicationEngine engine;
    MY_SPLASH_SET_PROGRESS(splash, 80);
    QQmlContext* context = engine.rootContext();
    context->setContextProperty("filterModelQml", &filterModel);
    context->setContextProperty("listModelQml", &listModel);

    QStringListModel netIfModel;
    MY_SPLASH_SET_PROGRESS(splash, 90);
    netIfModel.setStringList(udpfinder->ipaddr());
    context->setContextProperty("udpfinderModel", &netIfModel);
    context->setContextProperty("udpfinderQml", udpfinder);

    SystemTray * systemTray = new SystemTray();
    context->setContextProperty("systemTray", systemTray);
    context->setContextProperty("serviceUDP", &serviceUDP);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
    {
        return -1;
    }

    QThread::msleep(500);
    MY_SPLASH_SET_PROGRESS(splash, 100);
    MY_SPLASH_HIDE(splash);
    int ret = app.exec();
    qInfo() << fi.fileName() << " Closing!!!";
    systemTray->hideIconTray();

    return ret;
}
