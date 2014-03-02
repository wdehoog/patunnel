#include "pulse_stream.h"
#include "pulse_interface.h"

PulseStream::PulseStream(pa_sink_input_info const *stream_info, QObject *parent) :
    QObject(parent),
    m_name(stream_info->name),
    m_index(stream_info->index),
    m_sink_index(stream_info->sink)
{}

PulseStream::PulseStream(PulseStream const &s)
    : QObject(),
      m_name(s.name()),
      m_index(s.index()),
      m_sink_index(s.sink_index())
{}


void PulseStream::move_to_sink(QObject *sink) const
{
    PulseInterface *iface = qobject_cast<PulseInterface *>(PulseInterface::instance());
    iface->move_stream(*this, qobject_cast<PulseSink *>(sink));
}

PulseSink *PulseStream::sink() const
{
    PulseInterface *iface = qobject_cast<PulseInterface *>(PulseInterface::instance());
    return iface->find_sink_by_idx(sink_index());
}

bool PulseStream::operator ==(PulseStream const &stream)
{
    return m_index == stream.index();
}

PulseStream &PulseStream::operator= (PulseStream const &s) {
    if (m_name != s.name()) {
        m_name = s.name();
        emit name_changed();
    }
    m_index = s.index();

    if (m_sink_index != s.sink_index()) {
        m_sink_index = s.sink_index();
        emit sink_changed();
    }
    return *this;
}
