#include "pulse_sink.h"

/**
 * @brief PulseSink::PulseSink A representation of a PulseAudio sink (aka sound output, e.g. a sound card).
 * @param info PulseAudio's pa_sink_info struct that describes a sink.
 * @param parent Not used.
 */
PulseSink::PulseSink(pa_sink_info const * info, QObject *parent) :
    PulseObject(info->name, info->index, parent),
    m_description(info->description)
{}

PulseSink::PulseSink(PulseSink const &sink)
    : PulseObject(sink.name(), sink.index()),
      m_description(sink.description())
{}

QString PulseSink::description() const {
    QMutexLocker l(m_data_mutex);
    return m_description;
}

bool PulseSink::operator ==(PulseSink const &sink) {
    QMutexLocker l(m_data_mutex);
    return m_index == sink.index();
}

bool PulseSink::operator ==(PulseSink const *sink) {
    QMutexLocker l(m_data_mutex);
    return m_index == sink->index();
}

PulseSink &PulseSink::operator= (PulseSink const &sink)
{
    set_index(sink.index());
    set_name(sink.name());

    QMutexLocker l(m_data_mutex);
    if (m_description != sink.description()) {
        m_description = sink.description();
        emit description_changed();
    }
    return *this;
}
