import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.patunnel.Interface 1.0

Dialog {
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
        validator: RegExpValidator { regExp: /^((\d{1,3}\.){3}\d{1,3}(\/\d{1,2})?(;(\d{1,3}\.){3}\d{1,3}(\/\d{1,2})?)*)?$/ }
    }

    TextField {
        width: parent.width
        id: opts_field
        anchors { top: acl_field.bottom; left: parent.left; right: parent.right }
        label: "name=value ..."
        placeholderText: "Other options"
        validator: RegExpValidator { regExp: /^(\w+(=\S+)?(\s+\w+(=\S+)?)*)?$/ }
    }
}
