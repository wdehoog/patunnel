#include "pulse_sink.h"

PulseSink::PulseSink(pa_sink_info const * info, QObject *parent) :
    QObject(parent),
    m_description(info->description),
    m_index(info->index)
{
    m_name = (char *)calloc(strlen(info->name)+1, sizeof(char));
    strcpy(m_name, info->name);
}

PulseSink::PulseSink(PulseSink const &sink)
    : QObject(),
      m_description(sink.description()),
      m_index(sink.index())
{
    m_name = (char *)calloc(strlen(sink.name())+1, sizeof(char));
    strcpy(m_name, sink.name());
}

bool PulseSink::operator== (PulseSink const &sink) {
    return m_index == sink.index();
}

bool PulseSink::operator ==(PulseSink const *sink) {
    return m_index == sink->index();
}

PulseSink &PulseSink::operator= (PulseSink const &sink)
{
    m_description = sink.description();
    m_index = sink.index();

    m_name = (char *)calloc(strlen(sink.name())+1, sizeof(char));
    strcpy(m_name, sink.name());
    return *this;
}
