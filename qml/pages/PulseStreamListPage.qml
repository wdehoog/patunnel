import QtQuick 2.0
import Sailfish.Silica 1.0
import PaTunnel.Stream 1.0
import PaTunnel.Sink 1.0
import PaTunnel.Interface 1.0


Page {
    id: page
    onStatusChanged: { if(status === PageStatus.Active) { pageStack.pushAttached(Qt.resolvedUrl("PulseSinkListPage.qml")) }}

    SilicaListView {
        id: streamList
        model: PulseInterface.stream_list
        anchors.fill: parent
        header: PageHeader {
            title: "Pulseaudio Streams"
        }

        PullDownMenu {
            MenuItem {
                text: "Add tunnel sink"
                onClicked: pageStack.push(Qt.resolvedUrl("PulseAddTunnelPage.qml"))
            }
        }

        delegate: BackgroundItem {
            id: streamItem
            property PulseStream stream
            stream: this_stream

            ComboBox {
                id: streamComboBox
                label: name
                anchors.verticalCenter: parent.verticalCenter
                currentItem: streamItem.stream.sink

                menu: ContextMenu {
                    Repeater {
                        model: PulseInterface.sink_list
                        delegate: MenuItem {
                            text: description
                            onClicked: streamItem.stream.move_to_sink(this_sink)
                        }
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }
}


