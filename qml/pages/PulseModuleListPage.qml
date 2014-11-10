import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.patunnel.Module 1.0
import harbour.patunnel.Interface 1.0


Page {
    id: page
    RemorsePopup {
        id: remorse
    }

    SilicaListView {
        model: PulseInterface.module_list
        anchors.fill: parent
        header: PageHeader {
            title: "PulseAudio Modules"
        }

        PullDownMenu {
            MenuItem {
                text: "Load module"
                onClicked: pageStack.push(Qt.resolvedUrl("PulseLoadModulePage.qml"))
            }
            MenuItem {
                text: "Share local sound server"
                onClicked: pageStack.push(Qt.resolvedUrl("PulseAddTcpPage.qml"))
            }
        }

        delegate: ListItem {
            id: moduleItem
            ListView.onRemove: animateRemoval(listItem)
            enabled: true

            Label {
                id: nameLabel
                x: Theme.paddingLarge
                width: parent.width - 2*Theme.paddingLarge
                text: name.replace(/^module-/, "")
                color: moduleItem.enabled ? (moduleItem.highlighted ? Theme.highlightColor
                                                                    : Theme.primaryColor)
                                          : Theme.secondaryColor
                truncationMode: TruncationMode.Fade
            }
            Label {
                id: argLabel
                x: Theme.paddingLarge
                width: parent.width - 2*Theme.paddingLarge
                text: this_module.arguments
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                truncationMode: TruncationMode.Fade
                anchors.top: nameLabel.bottom
            }

            onClicked: pageStack.push(Qt.resolvedUrl("PulseModulePage.qml"), {
                                          pulse_module: this_module
                                      })

            menu: ContextMenu {
                MenuItem {
                    text: "Unload module"
                    x: Theme.paddingLarge
                    width: page.width - 2*Theme.paddingLarge
                    onClicked: remorse.execute("Unload" + name, function() {
                        PulseInterface.unload_module(this_module)
                    })
                }
            }
        }
        VerticalScrollDecorator {}
    }
}

