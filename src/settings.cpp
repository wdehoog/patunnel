#include "settings.h"

Settings::Settings(QObject *parent)
    : QSettings(QSettings::IniFormat, QSettings::UserScope, "harbour-patunnel", "harbour-patunnel", parent)
{}


Settings::AutoUnloadSetting Settings::policymodule_autounload()
{
    QVariant v = QSettings::value("autounload_policymodule", AUTOUNLOAD_ASK);
    return v.value<AutoUnloadSetting>();
}


void Settings::set_policymodule_autounload(AutoUnloadSetting autounload)
{
    QSettings::setValue("autounload_policymodule", autounload);
}

Settings *Settings::m_instance;

Settings *Settings::instance() {
    if (!m_instance) m_instance = new Settings();
    return m_instance;
}
