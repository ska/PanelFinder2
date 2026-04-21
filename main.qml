import QtQuick 2.12
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

ApplicationWindow {
    property string workingIp

    id: appWindow
    visible: true
    width: 500
    height: 300
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
            Qt.quit()
        }

        function onSignalIconActivated()
        {
            if(appWindow.visibility === 0 /*Window.Hidden*/)
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

    Connections {
        target: serviceUDP
        function onGuiMaximize()
        {
            appWindow.show();
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

    Popup {
        id: rebootPopup
        width: 280
        height: rebootColumn.implicitHeight + 16
        x: (appWindow.width  - width)  / 2
        y: (appWindow.height - height) / 2
        modal: true
        focus: true
        padding: 0
        closePolicy: Popup.NoAutoClose

        onOpened: rebootColumn.forceActiveFocus()

        background: Rectangle {
            color: "#333"
            border.color: "#888"
            radius: 4
        }

        Column {
            id: rebootColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 8
            spacing: 6
            focus: true

            Keys.onPressed: function(event) {
                if (event.key === Qt.Key_Escape) {
                    rebootPopup.close()
                } else if (event.key === Qt.Key_M) {
                    listModelQml.rebootMainOsPanel(workingIp)
                    workingIp = ""
                    rebootPopup.close()
                } else if (event.key === Qt.Key_C) {
                    listModelQml.rebootConfigOsPanel(workingIp)
                    workingIp = ""
                    rebootPopup.close()
                }
            }

            Text {
                text: "Reboot"
                font.pointSize: 9
                font.bold: true
                color: "white"
            }

            Rectangle { width: parent.width; height: 1; color: "#555" }

            Row {
                spacing: 5
                Text {
                    text: "Target"
                    font.pointSize: 9
                    color: "#8899bb"
                    width: 58
                }
                Text {
                    text: workingIp
                    font.pointSize: 9
                    font.bold: true
                    color: "#ddeeff"
                }
            }

            Text {
                text: "Select the OS to restart:"
                font.pointSize: 9
                color: "#8899bb"
            }

            Rectangle { width: parent.width; height: 1; color: "#444" }

            Row {
                spacing: 6
                anchors.horizontalCenter: parent.horizontalCenter

                Button {
                    text: "Config OS"
                    font.pointSize: 9
                    onClicked: {
                        listModelQml.rebootConfigOsPanel(workingIp)
                        workingIp = ""
                        rebootPopup.close()
                    }
                }
                Button {
                    text: "Main OS"
                    font.pointSize: 9
                    onClicked: {
                        listModelQml.rebootMainOsPanel(workingIp)
                        workingIp = ""
                        rebootPopup.close()
                    }
                }
                Button {
                    text: "Cancel"
                    font.pointSize: 9
                    onClicked: rebootPopup.close()
                }
            }
        }
    }

    Rectangle {
        anchors.top: parent.top;
        width: parent.width
        height: parent.height-40
        color: "gray"

        Image {
             anchors.fill: parent
             fillMode:  Image.Tile
             source: "qrc:/pics/easter-rabbit.png"
             opacity: 0.1
             visible: filterModelQml.getRandomNum() === 100 ? true : false;
         }

        GridView {
            id: grid
            objectName: "grid"
            width: parent.width
            height: parent.height
            anchors.top: parent.top;
            anchors.topMargin: 10
            anchors.fill: parent
            cellWidth: 260
            cellHeight: 80
            cacheBuffer: 100

            model: filterModelQml

            delegate: Item {
                id: cellItem
                objectName: "cellItem"
                width: 250
                height: 75

                PanelItem {
                    width: parent.width
                    height: parent.height
                    hostname: model.hostname
                    macaddr: model.macaddr
                    machine: model.machine
                    ipv4addr: model.ipv4addr
                    ipv4netmask: model.ipv4netmask
                    mainosVer: model.mainosVer
                    configosVer: model.configosVer
                    serialNo: model.serialNo
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
                    //stonkamUdpMulticast.testString( currentIndex )
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

