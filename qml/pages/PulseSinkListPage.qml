import QtQuick 2.0
import Sailfish.Silica 1.0
import PaTunnel.Sink 1.0
import PaTunnel.Interface 1.0


Page {
    id: page

    SilicaListView {
        PullDownMenu {
            MenuItem {
                text: "Add tunnel sink"
                onClicked: pageStack.push(Qt.resolvedUrl("PulseAddTunnelPage.qml"))
            }
        }

        id: sinkList
        model: PulseInterface.sink_list
        anchors.fill: parent
        header: PageHeader {
            title: "Sinks"
        }
        delegate: BackgroundItem {
            id: sinkItem
            enabled: index != PulseInterface.default_sink.index
            onClicked: PulseInterface.default_sink = this_sink

            Label {
                x: Theme.paddingLarge
                Text {
                    font.pixelSize: Theme.fontSizeSmall
                    color: sinkItem.enabled ? (sinkItem.highlighted ? Theme.highlightColor
                                                                    : Theme.primaryColor)
                                            : Theme.secondaryColor
                    text: "#" + index + ": " + description
                }
            }
        }
        VerticalScrollDecorator {}
    }
}

