import QtQuick 2.0
import Sailfish.Silica 1.0
import PaTunnel.Sink 1.0
import PaTunnel.Interface 1.0


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

            SinkLabel {
                theSink: this_sink
                enabled: sinkItem.enabled
                highlighted: sinkItem.highlighted
            }

            menu: ContextMenu {
                MenuItem {
                    text: "Set as default sink"
                    enabled: this_sink.index != PulseInterface.default_sink.index
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

