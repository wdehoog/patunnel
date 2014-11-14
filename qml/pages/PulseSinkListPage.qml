import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.patunnel.Sink 1.0
import harbour.patunnel.Interface 1.0
import harbour.patunnel.Settings 1.0

Page {
    RemorsePopup {
        id: remorse
    }

    SilicaListView {
        model: PulseInterface.sink_list
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            bottomMargin: autounload_question.visibleSize + Theme.paddingLarge*3
        }
        header: PageHeader {
            title: "PulseAudio Sinks"
        }

        PullDownMenu {
            MenuItem {
                text: "Load any module"
                onClicked: pageStack.push(Qt.resolvedUrl("PulseLoadModulePage.qml"))
            }
            MenuItem {
                text: "Share local sound server"
                onClicked: pageStack.push(Qt.resolvedUrl("PulseAddTcpPage.qml"))
            }
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
                    onClicked: {
                        var indices = PulseInterface.find_modules("module-policy-enforcement")
                        if (indices.length === 0) {
                            PulseInterface.default_sink = this_sink
                        }
                        else {
                            if (Settings.policymodule_autounload == Settings.AUTOUNLOAD_ASK) {
                                autounload_question.chosen_sink = this_sink
                                autounload_question.show()
                            }
                            else {
                                if (Settings.policymodule_autounload == Settings.AUTOUNLOAD_ALWAYS) {
                                    console.warn("Automaticall unloading module-policy-enforcement.")
                                    PulseInterface.unload_module("module-policy-enforcement")
                                }
                                if (Settings.policymodule_autounload == Settings.AUTOUNLOAD_NEVER) {
                                    console.warn("module-policy-enforcement NOT unloaded, setting default sink will have no effect.")
                                    PulseInterface.default_sink = this_sink
                                }
                            }
                        }
                    }
                }
                MenuItem {
                    text: "Unload module"
                    onClicked: remorse.execute("Unloading " + name, function() {
                        PulseInterface.unload_sink(this_sink)
                    })
                }
            }
        }
        VerticalScrollDecorator {}
    }

    DockedPanel {
        id: autounload_question
        width: parent.width
        height: help_label.height + unload_btn.height + setdflt_btn.height + save_switch.height + 4*Theme.paddingLarge
        dock: Dock.Bottom
        property PulseSink chosen_sink

        Column {
            x: Theme.paddingLarge
            width: parent.width - 2*Theme.paddingLarge
            spacing: Theme.paddingLarge


            Label {
                id: help_label
                text: "\nFor this to work, the policy enforcement\nmodule needs to be unloaded.\n"
                      + "As a result, phone calls and everything\nelse will also use this sink."
                anchors.topMargin: Theme.paddingLarge
                font.pixelSize: Theme.fontSizeSmall

                wrapMode: Text.Wrap
                maximumLineCount: 20
                truncationMode: TruncationMode.Fade
            }
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                id: unload_btn
                text: "Unload & set default"
                onClicked: {
                    if (save_switch.checked) {
                        Settings.policymodule_autounload = Settings.AUTOUNLOAD_ALWAYS
                    }
                    var indices = PulseInterface.find_modules("module-policy-enforcement");
                    if (indices.length > 1) {
                        console.warn("Strange, " + indices.length + " modules with the name module-policy-enforcement are loaded."
                                     + " There should be at most one. Proceed with caution.")
                    }
                    for (var i=0, module_idx; module_idx = indices[i++];) {
                        PulseInterface.unload_module(module_idx)
                    }
                    PulseInterface.default_sink = autounload_question.chosen_sink
                    autounload_question.hide()
                }
            }
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                id: setdflt_btn
                text: "Only set default"
                onClicked: {
                    if (save_switch.checked) {
                        Settings.policymodule_autounload = Settings.AUTOUNLOAD_NEVER
                    }
                    PulseInterface.default_sink = autounload_question.chosen_sink
                    autounload_question.hide()
                }
            }
            TextSwitch {
                id: save_switch
                text: "Don't ask again"
                checked: Settings.policymodule_autounload != Settings.AUTOUNLOAD_ASK
            }
        }
    }
}

