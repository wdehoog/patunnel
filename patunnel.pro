# The name of your app.
# NOTICE: name defined in TARGET has a corresponding QML filename.
#         If name defined in TARGET is changed, following needs to be
#         done to match new name:
#         - corresponding QML filename must be changed
#         - desktop icon filename must be changed
#         - desktop filename must be changed
#         - icon definition filename in desktop file must be changed
TARGET = patunnel

CONFIG += sailfishapp

SOURCES += src/patunnel.cpp \
    src/pulse_interface.cpp \
    src/pulse_object.cpp \
    src/pulse_stream.cpp \
    src/pulse_sink.cpp

QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_DEBUG += -ggdb

DEFINES += DEBUG_PULSE

OTHER_FILES += qml/patunnel.qml \
    qml/cover/CoverPage.qml \
    rpm/patunnel.spec \
    rpm/patunnel.yaml \
    patunnel.desktop \
    qml/pages/PulseSinkListPage.qml \
    qml/pages/PulseStreamListPage.qml \
    qml/pages/PulseAddTunnelPage.qml

HEADERS += \
    src/pulse_interface.h \
    src/pulse_object_list.h \
    src/pulse_stream.h \
    src/pulse_sink.h \
    src/pulse_object.h


unix: PKGCONFIG += libpulse
