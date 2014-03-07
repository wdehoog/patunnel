import QtQuick 2.0
import Sailfish.Silica 1.0
import PaTunnel.Sink 1.0


Column {
    id: sinkLabel

    property PulseSink theSink
    property var highlighted

    Label {
        id: descrLabel
        x: Theme.paddingLarge
        text: theSink.description
        color: sinkLabel.enabled ? (sinkLabel.highlighted ? Theme.highlightColor
                                                        : Theme.primaryColor)
                                : Theme.secondaryColor
    }
    Label {
        text: "#" + theSink.index + ": " + theSink.name
        color: descrLabel.color
        font.pixelSize: Theme.fontSizeExtraSmall
        x: Theme.paddingLarge
    }
}
