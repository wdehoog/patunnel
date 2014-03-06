#ifndef PULSE_SINK_H
#define PULSE_SINK_H

#include <QObject>
#include <pulse/introspect.h>

#include "pulse_object.h"

/**
 * @brief The PulseSink class
 */
class PulseSink : public PulseObject
{
    Q_OBJECT

    Q_PROPERTY(QString description READ description NOTIFY description_changed)
    Q_PROPERTY(PulseSink *this_sink READ this_sink)

public:
    explicit PulseSink(pa_sink_info const *info = NULL, QObject *parent = 0);
    PulseSink(PulseSink const &sink);

    QString description() const;
    PulseSink *this_sink() { return this; }
    unsigned int module_index() const;

    bool operator== (PulseSink const &o);
    bool operator== (PulseSink const *o);
    PulseSink &operator=(PulseSink const &sink);

private:
    QString m_description;
    unsigned int m_module_index;

signals:
    void description_changed();
};

#endif // PULSE_SINK_H
