import QtQuick 2.0
import QtQuick.Window 2.12
import QtQuick.Controls 2.12


Rectangle{
    id: panelItem;
    objectName: "panelItem"
    border.width: 1
    border.color: "#888"
    color: "lightgray"
    radius: 4
    smooth: true
    anchors.topMargin: 10;

    function getSettingsLink(panel)
    {
        if(panel.hostname == "Stonkam")
            return "http://"+panel.ipv4addr+"/"
        else
            return "https://"+panel.ipv4addr+"/machine_config/"
    }

    function getCameraSettingsLink(panel)
    {
        return "http://"+panel.ipv4addr+"/"
    }


    onParentChanged: {
        var panels = filterModelQml.rowCount();
        statusBarLabel.text = "Found " + panels + " devices"
        //console.log( parent.objectName )
        //console.log( statusBarLabel.text )
        systemTray.updateTooltip(panels)
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
    property string mainosVer
    property string configosVer
    property string serialNo

    Popup {
        id: credentialsPopup
        x: 0
        y: 0
        width: parent.width * 0.82
        height: credentialsColumn.implicitHeight + 16
        modal: true
        focus: true
        closePolicy: Popup.NoAutoClose
        padding: 0
        onOpened: passwordField.forceActiveFocus()

        background: Rectangle {
            color: "#333"
            border.color: "#888"
            radius: 4
        }

        Column {
            id: credentialsColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 8
            spacing: 6

            Text {
                text: "Credentials"
                font.pointSize: 9
                font.bold: true
                color: "white"
            }

            Rectangle { width: parent.width; height: 1; color: "#555" }

            Row {
                spacing: 5
                Text {
                    text: "IP"
                    font.pointSize: 9
                    color: "#8899bb"
                    width: 58
                }
                Text {
                    text: panelItem.ipv4addr
                    font.pointSize: 9
                    font.bold: true
                    color: "#ddeeff"
                }
            }
            Row {
                spacing: 5
                Text {
                    text: "Password"
                    font.pointSize: 9
                    color: "#8899bb"
                    width: 58
                }
                TextField {
                    id: passwordField
                    width: credentialsColumn.width - 58 - 5
                    height: 22
                    placeholderText: "password"
                    echoMode: TextInput.Password
                    color: "#ddeeff"
                    font.pointSize: 9
                    leftPadding: 4
                    background: Rectangle { color: "#555"; border.color: "#888"; radius: 2 }
                    Keys.onReturnPressed: {
                        listModelQml.savePanelCredentials(panelItem.ipv4addr, passwordField.text)
                        credentialsPopup.close()
                        passwordField.text = ""
                    }
                    Keys.onEscapePressed: {
                        credentialsPopup.close()
                        passwordField.text = ""
                    }
                }
            }

            Row {
                spacing: 6
                anchors.horizontalCenter: parent.horizontalCenter

                Button {
                    text: "Save"
                    font.pointSize: 9
                    onClicked: {
                        listModelQml.savePanelCredentials(panelItem.ipv4addr, passwordField.text)
                        credentialsPopup.close()
                        passwordField.text = ""
                    }
                }
                Button {
                    text: "Cancel"
                    font.pointSize: 9
                    onClicked: {
                        credentialsPopup.close()
                        passwordField.text = ""
                    }
                }
            }
        }
    }

    Popup {
        id: infoPopup
        x: 0
        y: 0
        width: parent.width * 0.82
        height: infoColumn.implicitHeight + 16
        modal: false
        focus: false
        opacity: 0.97
        closePolicy: Popup.NoAutoClose
        padding: 0

        background: Rectangle {
            color: "#333"
            border.color: "#888"
            radius: 4
        }

        Column {
            id: infoColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 8
            spacing: 6

            Text {
                text: "Device info"
                font.pointSize: 9
                font.bold: true
                color: "white"
            }

            Rectangle { width: parent.width; height: 1; color: "#555" }

            Row {
                spacing: 5
                Text {
                    text: "Main OS"
                    font.pointSize: 9
                    color: "#8899bb"
                    width: 58
                }
                Text {
                    text: panelItem.mainosVer
                    font.pointSize: 9
                    font.bold: true
                    color: "#ddeeff"
                }
            }
            Row {
                spacing: 5
                Text {
                    text: "Config OS"
                    font.pointSize: 9
                    color: "#8899bb"
                    width: 58
                }
                Text {
                    text: panelItem.configosVer
                    font.pointSize: 9
                    font.bold: true
                    color: "#ddeeff"
                }
            }
            Row {
                spacing: 5
                Text {
                    text: "SN"
                    font.pointSize: 9
                    color: "#8899bb"
                    width: 30
                }
                Text {
                    text: panelItem.serialNo
                    font.pointSize: 9
                    font.bold: true
                    color: "#ddeeff"
                    width: infoColumn.width - 35
                    elide: Text.ElideRight
                }
            }
        }
    }


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
                case "IPCamera":
                    settingsImageContainer.visible = true;
                    rebootImageContainer.visible = false;
                    infoImageContainer.visible = false;
                    copyipImageContainer.anchors.right = textContainer.right
                    settingsImageContainer.anchors.right = textContainer.right
                    return filterModelQml.getRandomNum() >= 95 ? "qrc:/pics/lakitu.png" : "qrc:/pics/ipcamera.png";

                default:
                    settingsImageContainer.visible = false;
                    rebootImageContainer.visible = false;
                    infoImageContainer.visible = false;
                    copyipImageContainer.anchors.right = textContainer.right
                    return filterModelQml.getRandomNum() >= 95 ? "qrc:/pics/mushV.png" : "qrc:/pics/linux.png";
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
            horizontalAlignment: Text.AlignLeft
            font.pointSize: 10
            color: "white"
            anchors.left: textContainer.left;
            anchors.leftMargin: 10;
            anchors.top: parent.top;
        }

        Text {
            id: macaddrText
            text: panelItem.macaddr;
            horizontalAlignment: Text.AlignLeft
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
            horizontalAlignment: Text.AlignLeft
            font.pointSize: 8
            color: "gray"
            anchors.left: textContainer.left;
            anchors.top: macaddrText.bottom;
            anchors.leftMargin: 10;
        }

        Text {
            id: ipv4addrText
            text: panelItem.ipv4addr + "/" + panelItem.ipv4netmask;
            horizontalAlignment: Text.AlignLeft
            font.pointSize: 8
            color: "gray"
            anchors.left: textContainer.left;
            anchors.top: machineText.bottom;
            anchors.leftMargin: 10;
        }


        Rectangle
        {
            id: infoImageContainer
            anchors.right: parent.right
            anchors.rightMargin: 3
            anchors.top: parent.top
            anchors.topMargin: 2
            height: 25
            width: height
            color: "transparent"

            Image {
                width: parent.width
                height: parent.height
                source: "qrc:/pics/info2GR.png"
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onEntered: { infoPopup.open() }
                    onExited:  { infoPopup.close() }
                    onClicked: function(mouse) {
                        if (mouse.button === Qt.RightButton) {
                            infoPopup.close()
                            credentialsPopup.open()
                        }
                    }
                }
            }
        }

        Rectangle
        {
            id: copyipImageContainer
            anchors.right: infoImageContainer.left
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


        Rectangle
        {
            id: rebootImageContainer
            anchors.right: parent.right
            anchors.rightMargin: 3
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 2
            height: 25
            width: height
            color: "transparent"
            visible: panelItem.mainosVer !== "" && panelItem.configosVer !== "" && panelItem.serialNo !== ""

            Image {
                width: parent.width
                height: parent.height
                source: "qrc:/pics/rebootGR.png"
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        workingIp = panelItem.ipv4addr
                        rebootPopup.open();
                    }
                    onEntered: {
                        panelItem.oldtext = statusBarLabel.text
                        statusBarLabel.text = "Reboot "+panelItem.ipv4addr
                    }
                    onExited: {
                        statusBarLabel.text = panelItem.oldtext;
                    }
                }
            }
        }


        Rectangle
        {
            id: settingsImageContainer
            anchors.right: rebootImageContainer.left
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
                    onClicked: { Qt.openUrlExternally(getSettingsLink(panelItem)) }

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

    }
}
