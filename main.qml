import QtQuick 2.4
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import QtQuick.Controls.Styles 1.2

ApplicationWindow {
    id: appWindow
    visible: true
    width: 750
    height: 500
    title: qsTr("Exor Panel finder " + "" + filterModelQml.getVersion() )

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
        anchors.bottom: parent.bottom;
        width: parent.width
        height: 40
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

    statusBar: StatusBar {
        RowLayout {
            height: 40;
            anchors.fill: parent
            Label {
                id: statusBarLabel
                text: "Waiting.. ("+filterModelQml.getRandomNum()+")"
            }
        }
    }
}
