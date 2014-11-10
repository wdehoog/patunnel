import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.patunnel.Interface 1.0

Dialog {
    canAccept: { ipField.text.length > 0
                 && sinkField.text.length > 0 }
    onAccepted: PulseInterface.load_module("module-tunnel-sink",
                    "server=" + ipField.text + " sink=" + sinkField.text)

    Column {
        anchors.fill: parent

        DialogHeader {
            acceptText: "Add tunnel sink"
        }
        TextField {
            width: parent.width
            id: ipField
            anchors { left: parent.left; right: parent.right }
            label: "Hostname or IP"
            placeholderText: label
            focus: true
            EnterKey.onClicked: {
                ipField.focus = false;
                sinkField.focus = true;
            }
            validator: RegExpValidator { regExp: /^\w+$/ }
        }

        TextField {
            width: parent.width
            id: sinkField
            anchors { left: parent.left; right: parent.right }
            label: "Sink name or index"
            placeholderText: label
            validator: RegExpValidator { regExp: /^.+/ }
        }
    }
}
