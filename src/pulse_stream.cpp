#include "pulse_stream.h"
#include "pulse_interface.h"

/**
 * @brief PulseStream::PulseStream A representation of a PulseAudio sink input.
 * @param stream_info The pa_sink_input_info struct used by PulseAudio to describe a sink input.
 * @param parent Not used.
 */
PulseStream::PulseStream(pa_sink_input_info const *stream_info, QObject *parent) :
    PulseObject(stream_info->name, stream_info->index, parent),
    m_sink_index(stream_info->sink)
{}

PulseStream::PulseStream(PulseStream const &s)
    : PulseObject(s.name(), s.index())
    , m_sink_index(s.sink_index())
{}

unsigned int PulseStream::sink_index() const {
    QMutexLocker l(m_data_mutex);
    return m_sink_index;
}

void PulseStream::move_to_sink(QObject *sink) const
{
    PulseInterface *iface = qobject_cast<PulseInterface *>(PulseInterface::instance());
    iface->move_stream(*this, qobject_cast<PulseSink *>(sink));
}

void PulseStream::unmute() const
{
    PulseInterface *iface = qobject_cast<PulseInterface *>(PulseInterface::instance());
    iface->unmute_stream(*this);
}

PulseSink *PulseStream::sink() const
{
    PulseInterface *iface = qobject_cast<PulseInterface *>(PulseInterface::instance());
    int idx = sink_list_idx();
    if (idx < 0) return NULL;
    else return &(iface->m_sinks[idx]);
}

int PulseStream::sink_list_idx() const
{
    QMutexLocker l(m_data_mutex);
    PulseInterface *iface = qobject_cast<PulseInterface *>(PulseInterface::instance());
    int idx = iface->m_sinks.find_pulse_index(sink_index());
    return idx;
}

bool PulseStream::operator ==(PulseStream const &stream)
{
    QMutexLocker l(m_data_mutex);
    return m_index == stream.index();
}

PulseStream &PulseStream::operator =(PulseStream const &s) {
    set_name(s.name());
    set_index(s.index());

    QMutexLocker l(m_data_mutex);
    if (m_sink_index != s.sink_index()) {
        m_sink_index = s.sink_index();
        emit sink_list_idx_changed();
        emit sink_changed();
        emit this_stream_changed();
    }
    return *this;
}

void PulseStream::this_changed() {
    emit this_stream_changed();
}
