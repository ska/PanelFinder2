import QtQuick 2.12
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

ApplicationWindow {
    property string workingIp

    id: appWindow
    visible: true
    width: 800
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
            width: 400
            height: 200
            x: (appWindow.width/2)-(width/2);
            y: (appWindow.height/2)-100;
            modal: true
            focus: false
            opacity: 0.95
            closePolicy: Popup.NoAutoClose

            GridLayout {
                id: rebootPopupGrid
                anchors.fill: parent
                rows: 2
                columns: 3

                Rectangle {
                     Layout.fillHeight: true
                     Layout.fillWidth: true
                     Layout.columnSpan: 3
                     Layout.rowSpan: 1
                     Layout.row: 0
                     Layout.column: 0
                     Text {
                         anchors.fill: parent
                         text: qsTr("<b>Confirm\n</n>Restart the system?")
                     }
                }

                Rectangle {
                     Layout.fillHeight: true
                     Layout.fillWidth: true
                     Layout.columnSpan: 1
                     Layout.rowSpan: 1
                     Layout.row: 1
                     Layout.column: 0

                     Button {
                         anchors.fill: parent
                         text: "ConfigOs"
                         onClicked: {
                             listModelQml.rebootConfigOsPanel( workingIp )
                             workingIp = ""
                             rebootPopup.close()
                         }
                     }
                }

                Rectangle {
                     Layout.fillHeight: true
                     Layout.fillWidth: true
                     Layout.columnSpan: 1
                     Layout.rowSpan: 1
                     Layout.row: 1
                     Layout.column: 1
                     Button {
                         anchors.fill: parent
                         text: "MainOs"
                         onClicked: {
                             listModelQml.rebootMainOsPanel( workingIp )
                             workingIp = ""
                             rebootPopup.close()
                         }
                     }
                }

               Rectangle {
                    color: "red"
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.columnSpan: 1
                    Layout.rowSpan: 1
                    Layout.row: 1
                    Layout.column: 2
                    Button {
                        anchors.fill: parent
                        text: "Cancel"
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

