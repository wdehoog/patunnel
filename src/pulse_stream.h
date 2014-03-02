#ifndef PULSE_STREAM_H
#define PULSE_STREAM_H

#include <QObject>
#include <pulse/introspect.h>
#include "pulse_sink.h"

class PulseStream : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name NOTIFY name_changed)
    Q_PROPERTY(unsigned int index READ index)
    Q_PROPERTY(PulseSink *sink READ sink NOTIFY sink_changed)
    Q_PROPERTY(PulseStream *this_stream READ this_stream)

public:
    explicit PulseStream(pa_sink_input_info const *stream_info = NULL, QObject *parent = 0);
    PulseStream(PulseStream const &s);

    QString name() const { return m_name; }
    unsigned int index() const { return m_index; }
    unsigned int sink_index() const { return m_sink_index; }
    PulseSink *sink() const;
    PulseStream *this_stream() { return this; }

    bool operator== (PulseStream const &s);
    PulseStream &operator= (PulseStream const &s);

    Q_INVOKABLE
    void move_to_sink(QObject *sink) const;

private:
    QString m_name;
    unsigned int m_index;
    unsigned int m_sink_index;

signals:
    void name_changed();
    void sink_changed();

public slots:
};

#endif // PULSE_STREAM_H
