#ifndef PULSE_MODULE_H
#define PULSE_MODULE_H

#include "pulse_object.h"

class PulseModule : public PulseObject
{
    Q_OBJECT

    Q_PROPERTY(PulseModule *this_module
               READ this_module
               NOTIFY this_module_changed)
public:
    PulseModule *this_module();

private:
    void this_changed();

signals:
    void this_module_changed();
};

#endif // PULSE_MODULE_H
