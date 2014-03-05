#ifndef PULSE_OBJECT_LIST_H
#define PULSE_OBJECT_LIST_H

#include <QList>
#include <QString>

template<class PulseObjectT>
class PulseObjectList : public QList<PulseObjectT>
{
public:
    int find_pulse_index(unsigned int pulse_index) const
    {
        int rv = 0;
        class PulseObjectList<PulseObjectT>::const_iterator it = PulseObjectList<PulseObjectT>::cbegin();
        for (; it != PulseObjectList<PulseObjectT>::cend(); ++it) {
            if (it->index() == pulse_index)
                return rv;
            rv++;
        }
        return -1;
    }


    int find_pulse_name(QString pulse_name) const
    {
        int rv = 0;
        class PulseObjectList<PulseObjectT>::const_iterator it = PulseObjectList<PulseObjectT>::cbegin();
        for (; it != PulseObjectList<PulseObjectT>::cend(); ++it) {
            if (it->name() == pulse_name)
                return rv;
            rv++;
        }
        return -1;
    }
};

#endif // PULSE_OBJECT_LIST_H
