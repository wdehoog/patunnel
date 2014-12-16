#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QMetaType>

QT_BEGIN_NAMESPACE

class Settings : public QSettings
{
    Q_OBJECT
    Q_ENUMS(AutoUnloadSetting)
    Q_PROPERTY(AutoUnloadSetting policymodule_autounload
               READ policymodule_autounload
               WRITE set_policymodule_autounload
               NOTIFY policymodule_autounload_changed)

public:
    explicit Settings(QObject *parent = 0);

    enum AutoUnloadSetting {
        AUTOUNLOAD_ASK = 0,
        AUTOUNLOAD_ALWAYS,
        AUTOUNLOAD_NEVER
    };
    Q_ENUMS(AutoUnloadSetting)

    static Settings *instance();

    AutoUnloadSetting policymodule_autounload();
    void set_policymodule_autounload(AutoUnloadSetting autounload);


private:
    static Settings *m_instance;


signals:
    void policymodule_autounload_changed();

};

Q_DECLARE_METATYPE(Settings::AutoUnloadSetting)

QT_END_NAMESPACE

#endif // SETTINGS_H
