# The name of your app.
# NOTICE: name defined in TARGET has a corresponding QML filename.
#         If name defined in TARGET is changed, following needs to be
#         done to match new name:
#         - corresponding QML filename must be changed
#         - desktop icon filename must be changed
#         - desktop filename must be changed
#         - icon definition filename in desktop file must be changed
TARGET = harbour-patunnel

CONFIG += sailfishapp

SOURCES += \
    src/pulse_interface.cpp \
    src/pulse_object.cpp \
    src/pulse_stream.cpp \
    src/pulse_sink.cpp \
    src/harbour-patunnel.cpp

QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_DEBUG += -ggdb

DEFINES += DEBUG_PULSE

OTHER_FILES += \
    qml/cover/CoverPage.qml \
    qml/pages/PulseSinkListPage.qml \
    qml/pages/PulseStreamListPage.qml \
    qml/pages/PulseAddTunnelPage.qml \
    harbour-patunnel.desktop \
    qml/harbour-patunnel.qml \
    rpm/harbour-patunnel.spec \
    rpm/harbour-patunnel.yaml

HEADERS += \
    src/pulse_interface.h \
    src/pulse_object_list.h \
    src/pulse_stream.h \
    src/pulse_sink.h \
    src/pulse_object.h

unix: PKGCONFIG += libpulse
