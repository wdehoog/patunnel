#ifndef PULSE_MODULE_H
#define PULSE_MODULE_H

#include "pulse_object.h"
#include <pulse/introspect.h>

class PulseModule : public PulseObject
{
    Q_OBJECT

    Q_PROPERTY(PulseModule *this_module
               READ this_module
               NOTIFY this_module_changed)

    Q_PROPERTY(QString arguments
               READ arguments
               NOTIFY arguments_changed)

    Q_PROPERTY(QString properties
               READ properties
               NOTIFY properties_changed)

public:
    explicit PulseModule(pa_module_info const *module = NULL, QObject *parent = 0);
    PulseModule(PulseModule const &other);

    PulseModule *this_module();

    QString arguments() const { return m_arguments; }
    QString properties() const { return m_properties; }

    bool operator ==(PulseModule const *module);
    bool operator ==(PulseModule const &module);
    bool operator !=(PulseModule const &module);
    PulseModule &operator =(PulseModule const &module);


private:
    void this_changed();
    QString m_properties;
    QString m_arguments;

signals:
    void this_module_changed();
    void arguments_changed();
    void properties_changed();
};

#endif // PULSE_MODULE_H
