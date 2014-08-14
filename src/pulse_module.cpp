#include "pulse_module.h"

PulseModule *PulseModule::this_module() {
    return this;
}

void PulseModule::this_changed() {
    emit this_module_changed();
}
