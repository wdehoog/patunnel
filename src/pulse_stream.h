#ifndef PULSE_STREAM_H
#define PULSE_STREAM_H

#include <QObject>
#include <pulse/introspect.h>

#include "pulse_sink.h"

/**
 * @brief The PulseStream class
 */
class PulseStream : public PulseObject
{
    Q_OBJECT

    Q_PROPERTY(PulseSink *sink READ sink NOTIFY sink_changed)
    Q_PROPERTY(int sink_list_idx READ sink_list_idx NOTIFY sink_list_idx_changed)
    Q_PROPERTY(PulseStream *this_stream READ this_stream NOTIFY this_changed)

public:
    explicit PulseStream(pa_sink_input_info const *stream_info = NULL, QObject *parent = 0);
    PulseStream(PulseStream const &s);
    unsigned int sink_index() const;
    PulseSink *sink() const;
    int sink_list_idx() const;

    PulseStream *this_stream() { return this; }

    bool operator== (PulseStream const &s);
    bool operator== (PulseStream const *s);
    PulseStream &operator= (PulseStream const &s);

    Q_INVOKABLE
    void move_to_sink(QObject *sink) const;

private:
    unsigned int m_sink_index;

signals:
    void this_changed();

    void sink_changed();
    void sink_list_idx_changed();
};


#endif // PULSE_STREAM_H
