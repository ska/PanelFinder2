import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ApplicationWindow {
    id: appWindow
    visible: true
    width: 750
    height: 500
    title: qsTr(filterModelQml.getName() + " - V" + filterModelQml.getVersion() )

    property bool ignoreCheck: false
    property bool showMessage: true

    Connections {
        target: systemTray
        function onSignalShow()
        {
            appWindow.show();
        }

        function onSignalQuit()
        {
            ignoreCheck = true
            close();
        }

        function onSignalIconActivated()
        {
            if(appWindow.visibility === Window.Hidden)
            {
                appWindow.show()
            } else {
                if(showMessage)
                {
                    showMessage =false
                    systemTray.showHideMessage()
                }
                appWindow.hide()
            }
        }

    }

    onClosing: {
        if(ignoreCheck === false)
        {
            if(showMessage)
            {
                showMessage =false
                systemTray.showHideMessage()
            }
            appWindow.hide()
        } else {
            Qt.quit()
        }
    }

    Rectangle {
        anchors.top: parent.top;
        width: parent.width
        height: parent.height-40
        color: "gray"

        GridView {
            id: grid
            width: parent.width
            height: parent.height
            anchors.top: parent.top;
            anchors.topMargin: 10
            anchors.fill: parent
            cellWidth: 250
            cellHeight: 80
            cacheBuffer: 100

            model: filterModelQml

            delegate: Item {
                id: cellItem
                width: 240
                height: 75

                PanelItem {
                    width: parent.width
                    height: parent.height
                    hostname: model.hostname
                    macaddr: model.macaddr
                    machine: model.machine
                    ipv4addr: model.ipv4addr
                    ipv4netmask: model.ipv4netmask
                }
            }
        }
    }

    Rectangle {
        //anchors.bottom: parent.bottom;
        anchors.bottom: statusBar.top
        width: parent.width
        height: 34
        color: "darkgray";
        z: 2

        RowLayout {
            id: rowLayout
            anchors.fill: parent
            anchors.centerIn: parent
            TextField {
                placeholderText: "Type MAC Addr here.."
                Layout.fillWidth: true
                font.pointSize: 12
                onTextChanged: {
                    filterModelQml.setFilterString(text);
                }
            }

            ComboBox {
                id: net
                height: parent.height
                Layout.fillWidth: true
                Layout.minimumWidth: 120
                Layout.preferredWidth: 130
                Layout.maximumWidth: 200
                model: udpfinderModel
                textRole: "display"

                onCurrentIndexChanged: {
                    udpfinderQml.testString( currentIndex )
                }
            }
        }
    }

    Rectangle {
        id: statusBar
        anchors.bottom: parent.bottom;
        width: parent.width
        height: 16
        color: "gray"
        smooth: true
        gradient:
            Gradient {
            GradientStop { position: 0.0; color: "gray" }
            GradientStop { position: 1.0; color: "darkgray" }
        }

        z: 2
        RowLayout {
            anchors.fill: parent
            anchors.centerIn: parent
            Label {
                id: statusBarLabel
                color: "white"
                text: "Waiting.. ("+filterModelQml.getRandomNum()+")"
            }
        }
    }
}

