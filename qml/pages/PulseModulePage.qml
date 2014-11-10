import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.patunnel.Module 1.0
import harbour.patunnel.Interface 1.0


Page {
    property PulseModule pulse_module
    id: module_page

    RemorsePopup {
        id: remorse
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: childrenRect.height

        VerticalScrollDecorator {}

        PullDownMenu {
            MenuItem {
                text: "Unload module"
                onClicked: remorse.execute("Unload " + pulse_module.name, function() {
                    PulseInterface.unload_module(pulse_module)
                    pageStack.pop()
                })
            }
        }

        PageHeader {
            id: mod_title
            title: "PulseAudio Module"
        }
        Label {
            id: name_label
            x: Theme.paddingLarge
            width: parent.width - 2*Theme.paddingLarge
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Theme.fontSizeLarge
            text: pulse_module.name
            anchors.top: mod_title.bottom
        }

        SectionHeader {
            id: prop_title
            text: "Properties"
            anchors.top: name_label.bottom
            truncationMode: TruncationMode.Fade
        }
        Label {
            id: prop_label
            text: pulse_module.properties
            x: Theme.paddingLarge
            width: parent.width - 2*Theme.paddingLarge
            font.pixelSize: Theme.fontSizeSmall
            wrapMode: Text.WordWrap
            maximumLineCount: 20
            truncationMode: TruncationMode.Fade
            anchors.top: prop_title.bottom
        }

        SectionHeader {
            id: arg_title
            text: "Arguments"
            anchors.top: prop_label.bottom
            truncationMode: TruncationMode.Fade
        }
        Label {
            id: arg_label
            text: pulse_module.arguments
            x: Theme.paddingLarge
            width: parent.width - 2*Theme.paddingLarge
            font.pixelSize: Theme.fontSizeSmall
            wrapMode: Text.Wrap
            maximumLineCount: 20
            truncationMode: TruncationMode.Fade
            anchors.top: arg_title.bottom
        }
    }

}
