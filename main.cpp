#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "panellistmodel.h"
#include "udpfinder.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
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

    QQmlApplicationEngine engine;
    QQmlContext* context = engine.rootContext();
    context->setContextProperty("filterModelQml", &filterModel);
    context->setContextProperty("pippo", &listModel);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
