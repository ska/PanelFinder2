import QtQuick 2.0
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

Rectangle{
    id: panelItem;
    border.width: 1
    border.color: "#888"
    color: "lightgray"
    radius: 4
    smooth: true
    anchors.topMargin: 10;

    onParentChanged: {
        statusBarLabel.text = "Found " + filterModelQml.rowCount() + " panels"
    }

    gradient:
        Gradient {
        GradientStop { position: 0.0; color: "#424242" }
        GradientStop { position: 1.0; color: "black" }
    }

    property string hostname
    property string macaddr
    property string machine
    property string ipv4addr
    property string ipv4netmask
    property string oldtext

    Rectangle
    {
        id: imageContainer
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 5;
        height: parent.height-20
        width: height
        color: "transparent"

        Image {
            width: parent.width
            height: parent.height
            source: {
                switch(panelItem.machine)
                {
                case "UN30":
                case "UN31":
                    return filterModelQml.getRandomNum() >= 95 ? "qrc:/pics/mushA.png" : "qrc:/pics/windows.png";
                case "UN63":
                case "UN73":
                    return filterModelQml.getRandomNum() >= 95 ? "qrc:/pics/mushGr.png" : "qrc:/pics/exn.png";
                case "UN66":
                    return filterModelQml.getRandomNum() >= 95 ? "qrc:/pics/mushO.png" : "qrc:/pics/exo.png";
                case "UN67":
                case "UN68":
                case "UN83":
                case "UN84":
                    return filterModelQml.getRandomNum() >= 95 ? "qrc:/pics/mushR.png" : "qrc:/pics/exr.png";
                case "UN75":
                    return filterModelQml.getRandomNum() >= 95 ? "qrc:/pics/mushV.png" : "qrc:/pics/on3r.png";
                default:
                    return filterModelQml.getRandomNum() >= 95 ? "qrc:/pics/mushB.png" : "qrc:/pics/exb.png";
                }
            }
        }
    }

    Rectangle
    {
        id: textContainer
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 5;
        height: parent.height-10
        width: parent.width-imageContainer.width-10
        color: "transparent"

        Text {
            id: hostnameText
            text: panelItem.hostname;
            horizontalAlignment: Text.left
            font.pointSize: 10
            color: "white"
            anchors.left: textContainer.left;
            anchors.leftMargin: 10;
            anchors.top: parent.top;
        }

        Text {
            id: macaddrText
            text: panelItem.macaddr;
            horizontalAlignment: Text.left
            font.pointSize: 8
            color: "gray"
            anchors.left: textContainer.left;
            anchors.top: hostnameText.bottom;
            anchors.leftMargin: 10;
            font.capitalization: Font.AllUppercase
        }

        Text {
            id: machineText
            text: panelItem.machine;
            horizontalAlignment: Text.left
            font.pointSize: 8
            color: "gray"
            anchors.left: textContainer.left;
            anchors.top: macaddrText.bottom;
            anchors.leftMargin: 10;
        }

        Text {
            id: ipv4addrText
            text: panelItem.ipv4addr + "/" + panelItem.ipv4netmask;
            horizontalAlignment: Text.left
            font.pointSize: 8
            color: "gray"
            anchors.left: textContainer.left;
            anchors.top: machineText.bottom;
            anchors.leftMargin: 10;
        }

        Rectangle
        {
            id: settingsImageContainer
            anchors.right: parent.right
            anchors.rightMargin: 5
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 2
            height: 25
            width: height
            color: "transparent"

            Image {
                width: parent.width
                height: parent.height
                source: "qrc:/pics/settingsGR.png"
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: { Qt.openUrlExternally("https://"+panelItem.ipv4addr+"/machine_config/") }
                    onEntered: {
                        panelItem.oldtext = statusBarLabel.text
                        statusBarLabel.text = "Open system settings on "+panelItem.ipv4addr
                    }
                    onExited: {
                        statusBarLabel.text = panelItem.oldtext;
                    }
                }
            }
        }

        Rectangle
        {
            id: copyipImageContainer
            anchors.right: parent.right
            anchors.rightMargin: 5
            anchors.top: parent.top
            anchors.topMargin: 2
            height: 25
            width: height
            color: "transparent"

            Image {
                width: parent.width
                height: parent.height
                source: "qrc:/pics/copyGR.png"
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: filterModelQml.copyIpToClipboard( panelItem.ipv4addr )
                    //onClicked: console.log( filterModelQml.rowCount() )
                    onEntered: {
                        panelItem.oldtext = statusBarLabel.text
                        statusBarLabel.text = "Copy "+panelItem.ipv4addr+" to clipboard"
                    }
                    onExited: {
                        statusBarLabel.text = panelItem.oldtext;
                    }
                }
            }
        }
    }
}
