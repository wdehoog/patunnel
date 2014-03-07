import QtQuick 2.0
import Sailfish.Silica 1.0
import PaTunnel.Stream 1.0
import PaTunnel.Sink 1.0
import PaTunnel.Interface 1.0


Page {
    id: page
    onStatusChanged: { if (status == PageStatus.Active) {
            pageStack.pushAttached(Qt.resolvedUrl("PulseSinkListPage.qml")) }
    }

    SilicaListView {
        id: streamList
        model: PulseInterface.stream_list

        anchors.fill: parent

        header: PageHeader {
            title: "PulseAudio Streams"
        }

        VerticalScrollDecorator {}

        PullDownMenu {
            MenuItem {
                text: "Add tunnel sink"
                onClicked: pageStack.push(Qt.resolvedUrl("PulseAddTunnelPage.qml"))
            }
            MenuItem {
                text: "Set default sink"
                onClicked: pageStack.navigateForward(PageStackAction.Animated)
            }
        }

        delegate: ComboBox {
            x: Theme.paddingLarge
            id: streamComboBox
            label: "#" + index + ": " + name

            menu: ContextMenu {
                Repeater {
                    model: PulseInterface.sink_list

                    delegate: MenuItem {
                        id: sinkItem
                        text: "#" + index + ": " + name
                        onClicked: this_stream.move_to_sink(this_sink)
                    }
                }
            }
            
            currentIndex: sink_list_idx
        }
    }
}
