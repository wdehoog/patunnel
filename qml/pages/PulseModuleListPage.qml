import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.patunnel.Module 1.0
import harbour.patunnel.Interface 1.0


Page {
    SilicaListView {
        model: PulseInterface.module_list
        anchors.fill: parent
        header: PageHeader {
            title: "PulseAudio Modules"
        }

        PullDownMenu {
            MenuItem {
                text: "Load module"
                onClicked: pageStack.push(Qt.resolvedUrl("PulseAddTunnelPage.qml"))
            }
        }

        delegate: ListItem {
            id: moduleItem
            ListView.onRemove: animateRemoval(listItem)
            enabled: true

            Column {
                Label {
                    id: descrLabel
                    x: Theme.paddingLarge
                    text: name.replace(/^module-/, "")
                    font.pixelSize: Theme.fontSizeSmall
                    color: moduleItem.enabled ? (moduleItem.highlighted ? Theme.highlightColor
                                                                    : Theme.primaryColor)
                                            : Theme.secondaryColor
                    truncationMode: TruncationMode.Fade
                }
                Label {
                    text: this_module.arguments
                    color: descrLabel.color
                    font.pixelSize: Theme.fontSizeExtraSmall
                    x: Theme.paddingLarge
                    truncationMode: TruncationMode.Fade
                }
            }

            menu: ContextMenu {
                Label {
                    x: Theme.paddingLarge
                    text: properties
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    truncationMode: TruncationMode.Fade
                }
                MenuItem {
                    text: "Unload module"
                    onClicked: PulseInterface.unload_module(this_module)
                }
            }
        }
        VerticalScrollDecorator {}
    }
}

