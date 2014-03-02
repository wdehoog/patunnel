#ifndef PULSE_SINK_H
#define PULSE_SINK_H

#include <QObject>
#include <pulse/introspect.h>

class PulseSink : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString description READ description)
    Q_PROPERTY(unsigned int index READ index)
    Q_PROPERTY(PulseSink *this_sink READ this_sink)

public:
    explicit PulseSink(pa_sink_info const *info = NULL, QObject *parent = 0);
    PulseSink(PulseSink const &sink);

    QString description() const { return m_description; }
    char const *name() const { return m_name; }
    unsigned int index() const { return m_index; }
    PulseSink *this_sink() { return this; }

    bool operator== (PulseSink const &o);
    PulseSink &operator=(PulseSink const &sink);

private:
    QString m_description;
    char *m_name;
    unsigned int m_index;

signals:

public slots:

};

#endif // PULSE_SINK_H
