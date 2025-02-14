import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Esterv.Styles.Simple
import Esterv.CustomControls
import Esterv.Crypto.Vault

ApplicationWindow {
    id: window
    visible: true

    background: Rectangle {
        color: Style.backColor1
    }

    Popup {
        id: pop1
        property bool isSet: false

        visible: false
        closePolicy: Popup.CloseOnPressOutside
        anchors.centerIn: Overlay.overlay
        onClosed: {
            pass.text = "";
        }

        ColumnLayout {
            Label {
                text: qsTr(((vault.isEmpty) ? "Set" : "Enter") + " the password:")
            }
            TextField {
                id: pass
                Layout.fillWidth: true
                echoMode: TextInput.PasswordEchoOnEdit
                ToolTip.text: qsTr("Wrong password")
                ToolTip.visible: false
            }

            Button {
                text: qsTr("ok")
                enabled: pass.text.length > 8
                onClicked: {
                    if (pop1.isSet) {
                        if (!vault.setDataString(pass.text, setdata.text)) {
                            pass.ToolTip.show(qsTr("Wrong password"), 5000);
                        } else {
                            pop1.close();
                        }
                    } else {
                        getdata.text = vault.getDataString(pass.text);
                        pop1.close();
                    }
                }
                Layout.alignment: Qt.AlignRight || Qt.AlignBottom
            }
        }
    }
    Popup {
        id: pop2
        visible: false
        closePolicy: Popup.CloseOnPressOutside
        anchors.centerIn: Overlay.overlay

        onClosed: {
            oldpass.text = "";
            newpass.text = "";
        }

        ColumnLayout {
            Label {
                text: qsTr("Enter the vault password:")
            }
            TextField {
                id: oldpass
                Layout.fillWidth: true
                echoMode: TextInput.PasswordEchoOnEdit
            }
            Label {
                text: qsTr("Set the vault password:")
            }
            TextField {
                id: newpass
                Layout.fillWidth: true
                echoMode: TextInput.PasswordEchoOnEdit
            }

            Button {
                text: qsTr("ok")
                enabled: (newpass.text.length > 8) && (oldpass.text.length > 8)
                onClicked: {
                    if (vault.changePassword(oldpass.text, newpass.text)) {
                        pop2.close();
                    } else {
                        oldpass.ToolTip.show(qsTr("Wrong password"), 5000);
                    }
                }
                Layout.alignment: Qt.AlignRight || Qt.AlignBottom
            }
        }
    }

    Vault {
        id: vault
    }
    ThemeSwitch {
        id: themeswitch
    }

    ColumnLayout {
        width: parent.width
        height: parent.height - themeswitch.height
        anchors.top: themeswitch.bottom
        spacing: 30
        RowLayout {
            Layout.alignment: Qt.AlignHCenter || Qt.AlignTop
            Label {
                text: qsTr("The vault is " + ((vault.isEmpty) ? "" : "not") + " empty")
            }
            Button {
                id: changepass
                Layout.margins: 5
                text: qsTr("Change password")
                visible: !vault.isEmpty
                onClicked: {
                    pop2.open();
                }
            }
        }

        GridLayout {

            flow: (parent.width < 600) ? GridLayout.TopToBottom : GridLayout.LeftToRight
            columns: 2
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: 40
            columnSpacing: 30
            rowSpacing: 30
            Layout.alignment: Qt.AlignCenter
            Rectangle {
                color: Style.backColor2
                Layout.minimumHeight: 200
                Layout.maximumHeight: 300
                Layout.minimumWidth: 300
                Layout.maximumWidth: 400
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                ColumnLayout {
                    anchors.fill: parent
                    Label {
                        text: qsTr("Set the data:")
                        Layout.margins: 5
                    }
                    TextArea {
                        id: setdata
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.maximumWidth: 300
                        Layout.maximumHeight: 200
                        Layout.alignment: Qt.AlignCenter
                    }
                    Button {
                        text: qsTr("Save")
                        Layout.margins: 5
                        onClicked: {
                            pop1.isSet = true;
                            pop1.open();
                        }
                        Layout.alignment: Qt.AlignRight || Qt.AlignBottom
                    }
                }
            }
            Rectangle {
                color: Style.backColor2
                Layout.minimumHeight: 200
                Layout.maximumHeight: 300
                Layout.minimumWidth: 300
                Layout.maximumWidth: 400
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                ColumnLayout {
                    anchors.fill: parent
                    Label {
                        Layout.margins: 5
                        text: qsTr("data:")
                    }
                    TextArea {
                        id: getdata
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.maximumWidth: 300
                        Layout.maximumHeight: 200
                        Layout.alignment: Qt.AlignCenter
                        readOnly: true
                    }

                    Button {
                        text: qsTr("Retrieve")
                        Layout.margins: 5
                        onClicked: {
                            pop1.isSet = false;
                            pop1.open();
                        }
                        Layout.alignment: Qt.AlignRight || Qt.AlignBottom
                    }
                }
            }
        }
    }
}
