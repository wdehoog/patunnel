#ifndef PULSE_OBJECTS_H
#define PULSE_OBJECTS_H

#include <QObject>
#include <QMutexLocker>

/**
 * @brief The PulseObject class
 */
class PulseObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name NOTIFY name_changed)
    Q_PROPERTY(unsigned int index READ index NOTIFY index_changed)

public:
    explicit PulseObject(QString name, unsigned int index, QObject *parent = 0);
    ~PulseObject();

    QString name() const;
    unsigned int index() const;

protected:
    void set_name(QString name);
    void set_index(unsigned int index);
    QMutex *m_data_mutex;
    QString m_name;
    unsigned int m_index;

signals:
    void name_changed();
    void index_changed();
};


#endif // PULSE_OBJECTS_H
