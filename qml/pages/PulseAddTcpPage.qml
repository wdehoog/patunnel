import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.patunnel.Interface 1.0

Dialog {
    canAccept: { ipField.text.length > 0
                 && sinkField.text.length > 0 }
    onAccepted: PulseInterface.add_tunnel_sink(ipField.text, sinkField.text)

    Column {
        anchors.fill: parent

        DialogHeader {
            acceptText: "Share local sound server"
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
