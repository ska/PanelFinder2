TEMPLATE = app

QT += qml quick

SOURCES += main.cpp \
    panellistmodel.cpp \
    udpfinder.cpp

RESOURCES += qml.qrc

HEADERS += \
    panellistmodel.h \
    udpfinder.h

DISTFILES += \
    pics/copy.png \
    pics/copyGR.png \
    pics/exb.png \
    pics/exn.png \
    pics/exo.png \
    pics/exr.png \
    pics/mushA.png \
    pics/mushB.png \
    pics/mushG.png \
    pics/mushGr.png \
    pics/mushO.png \
    pics/mushR.png \
    pics/mushV.png \
    pics/settingsGR.png \
    pics/settingsW.png


# Default rules for deployment.
target.path = /home/admin
INSTALLS += target

RC_ICONS = "icon.ico"
