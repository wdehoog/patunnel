import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.patunnel.Interface 1.0

Dialog {
    id: dialog
    onAccepted: PulseInterface.load_module("module-native-protocol-tcp",
                                           acl_field.text != "" ? "auth-ip-acl=" + acl_field.text : ""
                                           + opts_field.text)

    DialogHeader {
        id: header
        acceptText: "Share server"
    }

    TextField {
        width: parent.width
        id: acl_field
        anchors { top: header.bottom; left: parent.left; right: parent.right }
        label: "IPs/networks separated by semicolon"
        placeholderText: "Access list"
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
        EnterKey.onClicked: {
            acl_field.focus = false;
            opts_field.focus = true;
        }
        EnterKey.enabled: text.length > 0
        EnterKey.iconSource: "image://theme/icon-m-enter-next"
        validator: RegExpValidator { regExp: /^((\d{1,3}\.){3}\d{1,3}(\/\d{1,2})?(;(\d{1,3}\.){3}\d{1,3}(\/\d{1,2})?)*)?$/ }
    }

    TextField {
        width: parent.width
        id: opts_field
        anchors { top: acl_field.bottom; left: parent.left; right: parent.right }
        label: "name=value ..."
        placeholderText: "Other options"
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
        EnterKey.iconSource: "image://theme/icon-m-enter-accept"
        EnterKey.onClicked: dialog.accept()
        validator: RegExpValidator { regExp: /^(\w+(=\S+)?(\s+\w+(=\S+)?)*)?$/ }
    }
}
