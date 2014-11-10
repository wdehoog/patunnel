import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.patunnel.Interface 1.0

Dialog {
    canAccept: name_field.text.length > 0
    onAccepted: {
        PulseInterface.load_module(name_field.text, arg_field.text)
    }
    Column {
        anchors.fill: parent

        DialogHeader {
            acceptText: "Load module"
        }
        TextField {
            width: parent.width
            id: name_field
            anchors { left: parent.left; right: parent.right }
            label: "Module name"
            placeholderText: label
            focus: true
            EnterKey.onClicked: {
                name_field.focus = false;
                arg_field.focus = true;
            }
            validator: RegExpValidator { regExp: /^\w+$/ }
        }

        TextField {
            width: parent.width
            id: arg_field
            anchors { left: parent.left; right: parent.right }
            label: "name[=value] ..."
            placeholderText: label
            validator: RegExpValidator { regExp: /^(\w+(=\S+)?(\s+\w+(=\S+)?)*)?$/ }
        }
    }
}
