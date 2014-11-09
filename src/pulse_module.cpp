#include "pulse_module.h"

PulseModule::PulseModule(PulseModule const &other)
    : PulseObject(other.name(), other.index(), other.parent()),
      m_properties(other.properties()),
      m_arguments(other.arguments())
{}

PulseModule::PulseModule(const pa_module_info *module, QObject *parent)
    : PulseObject(module->name, module->index, parent),
      m_properties(pa_proplist_to_string(module->proplist)),
      m_arguments(module->argument)
{}

PulseModule *PulseModule::this_module() {
    return this;
}

void PulseModule::this_changed() {
    emit this_module_changed();
}

bool PulseModule::operator ==(PulseModule const *module) {
    QMutexLocker l(m_data_mutex);
    return m_index == module->index();
}

bool PulseModule::operator ==(PulseModule const &module) {
    QMutexLocker l(m_data_mutex);
    return m_index == module.index();
}

bool PulseModule::operator !=(PulseModule const &module) {
    return !(*this == module);
}

PulseModule &PulseModule::operator =(PulseModule const &module) {
    bool change = *this != module;
    set_index(module.index());
    set_name(module.name());

    if (m_properties != module.properties()) {
        m_properties = module.properties();
        emit properties_changed();
        change = true;
    }
    if (m_arguments != module.arguments()) {
        m_arguments = module.arguments();
        emit arguments_changed();
        change = true;
    }

    if (change) this_changed();
    return *this;
}
