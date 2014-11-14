import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.patunnel.Interface 1.0

Dialog {
    id: dialog
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
            EnterKey.enabled: text.length > 0
            EnterKey.iconSource: "image://theme/icon-m-enter-next"
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
            validator: RegExpValidator { regExp: /^\S+$/ }
        }

        TextField {
            width: parent.width
            id: arg_field
            anchors { left: parent.left; right: parent.right }
            label: "name[=value] ..."
            placeholderText: label
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
            EnterKey.iconSource: "image://theme/icon-m-enter-accept"
            EnterKey.onClicked: dialog.accept()
            validator: RegExpValidator { regExp: /^(\w+(=\S+)?(\s+\w+(=\S+)?)*)?$/ }
        }
    }
}
