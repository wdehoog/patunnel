#include "pulse_object.h"

/**
 * @brief PulseObject::PulseObject An abstract model of the PulseAudio objects we use.
 * @param name The unique PulseAudio name of the object.
 * @param index The PulseAudio index, unique for a certain object type.
 * @param parent Not used.
 */
PulseObject::PulseObject(QString name, unsigned int index, QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_index(index)
{
    m_data_mutex = new QMutex(QMutex::Recursive);
}

PulseObject::~PulseObject()
{
    delete m_data_mutex;
}

QString PulseObject::name() const {
    QMutexLocker l(m_data_mutex);
    return m_name;
}
unsigned int PulseObject::index() const {
    QMutexLocker l(m_data_mutex);
    return m_index;
}

void PulseObject::set_name(QString name) {
    if (m_name != name) {
        QMutexLocker l(m_data_mutex);
        m_name = name;
        emit name_changed();
    }
}

void PulseObject::set_index(unsigned int index) {
    if (m_index != index) {
        QMutexLocker l(m_data_mutex);
        m_index = index;
        emit index_changed();
    }
}
