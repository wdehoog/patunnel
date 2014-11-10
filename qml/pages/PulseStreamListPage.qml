import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.patunnel.Stream 1.0
import harbour.patunnel.Sink 1.0
import harbour.patunnel.Interface 1.0


Page {
    id: page

    SilicaListView {
        id: streamList
        model: PulseInterface.stream_list

        anchors.fill: parent

        header: PageHeader {
            title: "PulseAudio Streams"
        }

        VerticalScrollDecorator {}

        ViewPlaceholder {
            enabled: streamList.count == 0
            text: "No streams"
            hintText: "No application is currently playing audio."
        }


        PullDownMenu {
            MenuItem {
                text: "Sinks"
                onClicked: pageStack.push(Qt.resolvedUrl("PulseSinkListPage.qml"))
            }
            MenuItem {
                text: "Modules"
                onClicked: pageStack.push(Qt.resolvedUrl("PulseModuleListPage.qml"))
            }

            // UI feedback sounds from opening the pulley menu would change the stream list,
            // which would update the list view and slam the pulley menu shut while the user
            // is interacting with it. Therefore, stream list updates are deferred until the
            // menu is closed again.
            // cf. https://github.com/vmatare/patunnel/issues/1
            onActiveChanged: PulseInterface.defer_stream_list_updates(active)
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
                        onClicked: move_to_sink(this_sink)
                        x: Theme.paddingLarge
                        truncationMode: TruncationMode.Fade
                    }
                }
            }
            
            currentIndex: sink_list_idx
        }
    }
}
