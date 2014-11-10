import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.patunnel.Sink 1.0
import harbour.patunnel.Interface 1.0


Page {
    SilicaListView {
        model: PulseInterface.sink_list
        anchors.fill: parent
        header: PageHeader {
            title: "PulseAudio Sinks"
        }

        PullDownMenu {
            MenuItem {
                text: "Add tunnel sink"
                onClicked: pageStack.push(Qt.resolvedUrl("PulseAddTunnelPage.qml"))
            }
        }

        delegate: ListItem {
            id: sinkItem
            ListView.onRemove: animateRemoval(listItem)
            enabled: index != PulseInterface.default_sink.index

            Column {
                Label {
                    id: descrLabel
                    x: Theme.paddingLarge
                    text: description
                    color: sinkItem.enabled ? (sinkItem.highlighted ? Theme.highlightColor
                                                                    : Theme.primaryColor)
                                            : Theme.secondaryColor
                }
                Label {
                    text: "#" + index + ": " + name
                    color: descrLabel.color
                    font.pixelSize: Theme.fontSizeExtraSmall
                    x: Theme.paddingLarge
                }
            }

            menu: ContextMenu {
                MenuItem {
                    text: "Set as default sink"
                    enabled: index != PulseInterface.default_sink.index
                    onClicked: PulseInterface.default_sink = this_sink
                }
                MenuItem {
                    text: "Unload module"
                    onClicked: PulseInterface.unload_sink(this_sink)
                }
            }
        }
        VerticalScrollDecorator {}
    }
}

