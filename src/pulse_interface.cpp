#include <QDebug>
#include <QMutexLocker>
#include <QCoreApplication>

#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "pulse_interface.h"

QT_BEGIN_NAMESPACE


#define _COMPLETE_PA_OP(_pa_iface_op_func) \
    pa_operation *_pa_iface_op = _pa_iface_op_func; \
    while (pa_operation_get_state(_pa_iface_op) == PA_OPERATION_RUNNING) \
        pa_threaded_mainloop_wait(m_mainLoop); \
    pa_operation_unref(_pa_iface_op); \
    pa_threaded_mainloop_unlock(m_mainLoop);


static void cb_server_info(pa_context *context, const pa_server_info *info, void *userdata)
{
    PulseInterface *pulseEngine = static_cast<PulseInterface*>(userdata);

    if (!info) {
        qWarning() << QString("Failed to get server information: %s").arg(
                          pa_strerror(pa_context_errno(context)));
    }
    else if (!pulseEngine->m_default_sink ||
             (pulseEngine->m_default_sink->name() != info->default_sink_name)) {
        QMutexLocker(&(pulseEngine->m_data_mutex));
        int idx = pulseEngine->m_sinks.find_pulse_name(info->default_sink_name);
        if (idx >= 0) {
            pulseEngine->m_default_sink = &pulseEngine->m_sinks[idx];
            pulseEngine->m_default_sink->moveToThread(QCoreApplication::instance()->thread());

            emit pulseEngine->default_sink_changed();
            emit pulseEngine->sink_list_changed();
            qDebug() << "Default sink:" << info->default_sink_name;
        }
    }
    pa_threaded_mainloop_signal(pulseEngine->mainloop(), 0);
}


static void cb_sink_info(
        pa_context *context, const pa_sink_info *sink, int isLast, void *userdata)
{
    PulseInterface *pulseEngine = static_cast<PulseInterface*>(userdata);

    if (isLast < 0) {
        qWarning() << QString("Failed to get sink information: %s").arg(
                          pa_strerror(pa_context_errno(context)));
        return;
    }

    if (isLast) {
        pa_threaded_mainloop_signal(pulseEngine->mainloop(), 0);
        if (pulseEngine->m_sinks_changed) {
            emit pulseEngine->sink_list_changed();
            pulseEngine->m_sinks_changed = false;
        }
        return;
    }

    Q_ASSERT(sink);

    QMutexLocker(&(pulseEngine->m_data_mutex));
    PulseSink pulse_sink(sink);
    pulse_sink.moveToThread(QCoreApplication::instance()->thread());
    int idx = pulseEngine->m_sinks.indexOf(pulse_sink);
    if (idx < 0) {
        pulseEngine->m_sinks.append(pulse_sink);
        pulseEngine->m_sinks_changed = true;
    }
    else {
        pulseEngine->m_sinks.replace(idx, pulse_sink);
    }
}


static void cb_context_state_init(pa_context *context, void *userdata)
{
    Q_UNUSED(context);

    PulseInterface *pulseEngine = reinterpret_cast<PulseInterface*>(userdata);
    pa_threaded_mainloop_signal(pulseEngine->mainloop(), 0);
}


static void cb_context_state(pa_context *context, void *userdata)
{
    Q_UNUSED(userdata);
    Q_UNUSED(context);

}


static void cb_stream_info(pa_context *context, const pa_sink_input_info *stream, int eol, void *userdata)
{
    PulseInterface *pulseEngine = reinterpret_cast<PulseInterface*>(userdata);

    if (eol < 0) {
        qWarning() << QString("Failed to get stream information: %s").arg(
                          pa_strerror(pa_context_errno(context)));
        return;
    }

    if (eol) {
        pa_threaded_mainloop_signal(pulseEngine->mainloop(), 0);
        if (pulseEngine->m_streams_changed) {
            emit pulseEngine->stream_list_changed();
            pulseEngine->m_streams_changed = false;
        }
        return;
    }

    Q_ASSERT(stream);
    QMutexLocker(&(pulseEngine->m_data_mutex));

    PulseStream pulse_stream(stream);
    pulse_stream.moveToThread(QCoreApplication::instance()->thread());
    int idx = pulseEngine->m_streams.indexOf(pulse_stream);
    if (idx < 0) {
        pulseEngine->m_streams.append(pulse_stream);
        pulseEngine->m_streams_changed = true;
    }
    else {
        pulseEngine->m_streams.replace(idx, pulse_stream);
    }
}


static void cb_subscribe(pa_context *c, pa_subscription_event_type_t t, uint32_t pa_idx, void *userdata)
{
    Q_UNUSED(c);
    PulseInterface *pulse_iface = reinterpret_cast<PulseInterface*>(userdata);
    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
    case PA_SUBSCRIPTION_EVENT_SINK:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            QMutexLocker(&(pulse_iface->m_data_mutex));
            int list_idx = pulse_iface->m_sinks.find_pulse_index(pa_idx);
            if (list_idx >= 0) {
                pulse_iface->m_sinks.removeAt(list_idx);
                emit pulse_iface->sink_list_changed();
            }
        }
        else {
            pa_operation *operation = pa_context_get_sink_info_by_index(
                        pulse_iface->context(), pa_idx, cb_sink_info, userdata);
            if (!operation)
                emit pulse_iface->runtime_error(QString("Error getting info for sink #%1").arg(pa_idx));
            pa_operation_unref(operation);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            QMutexLocker(&(pulse_iface->m_data_mutex));
            int list_idx = pulse_iface->m_streams.find_pulse_index(pa_idx);
            if (list_idx >= 0) {
                pulse_iface->m_streams.removeAt(list_idx);
                emit pulse_iface->stream_list_changed();
            }
        }
        else {
            pa_operation *operation = pa_context_get_sink_input_info(pulse_iface->context(), pa_idx, cb_stream_info, userdata);
            if (!operation)
                emit pulse_iface->runtime_error(QString("Error getting info for stream #%1").arg(pa_idx));
            pa_operation_unref(operation);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SERVER:
        pa_operation *op = pa_context_get_server_info(pulse_iface->context(), cb_server_info, pulse_iface);
        if (!op)
            emit pulse_iface->runtime_error(QString("Error getting server info."));
        break;
    }
}

Q_GLOBAL_STATIC(PulseInterface, the_instance)

PulseInterface::PulseInterface(QObject *parent)
    : QObject(parent)
    , m_default_sink(NULL)
    , m_data_mutex(QMutex::Recursive)
    , m_mainLoopApi(0)
    , m_context(0)
{
    bool keepGoing = true;
    bool ok = true;

    m_mainLoop = pa_threaded_mainloop_new();
    if (m_mainLoop == 0) {
        qWarning("Unable to create pulseaudio mainloop");
        return;
    }

    if (pa_threaded_mainloop_start(m_mainLoop) != 0) {
        qWarning("Unable to start pulseaudio mainloop");
        pa_threaded_mainloop_free(m_mainLoop);
        return;
    }

    m_mainLoopApi = pa_threaded_mainloop_get_api(m_mainLoop);

    pa_threaded_mainloop_lock(m_mainLoop);

    m_context = pa_context_new(m_mainLoopApi, QString(QLatin1String("QtmPulseContext:%1")).arg(::getpid()).toLatin1().constData());
    pa_context_set_state_callback(m_context, cb_context_state_init, this);

    if (!m_context) {
        qWarning("Unable to create new pulseaudio context");
        pa_threaded_mainloop_free(m_mainLoop);
        return;
    }

    if (pa_context_connect(m_context, NULL, (pa_context_flags_t)0, NULL) < 0) {
        qWarning("Unable to create a connection to the pulseaudio context");
        pa_context_unref(m_context);
        pa_threaded_mainloop_free(m_mainLoop);
        return;
    }

    pa_threaded_mainloop_wait(m_mainLoop);

    while (keepGoing) {
        switch (pa_context_get_state(m_context)) {
            case PA_CONTEXT_CONNECTING:
            case PA_CONTEXT_AUTHORIZING:
            case PA_CONTEXT_SETTING_NAME:
                break;

            case PA_CONTEXT_READY:
#ifdef DEBUG_PULSE
                qDebug("Connection established.");
#endif
                keepGoing = false;
                break;

            case PA_CONTEXT_TERMINATED:
                qCritical("Context terminated.");
                keepGoing = false;
                ok = false;
                break;

            case PA_CONTEXT_FAILED:
            default:
                qCritical() << QString("Connection failure: %1").arg(pa_strerror(pa_context_errno(m_context)));
                keepGoing = false;
                ok = false;
        }

        if (keepGoing) {
            pa_threaded_mainloop_wait(m_mainLoop);
        }
    }

    if (ok) {
        pa_context_set_state_callback(m_context, cb_context_state, this);
    } else {
        if (m_context) {
            pa_context_unref(m_context);
            m_context = 0;
        }
    }

    pa_threaded_mainloop_unlock(m_mainLoop);

    if (ok) {
        get_sinks();
        get_streams();
        subscribe();
        get_server_info();
    }
}


PulseInterface::~PulseInterface()
{
    if (m_context) {
        pa_threaded_mainloop_lock(m_mainLoop);
        pa_context_disconnect(m_context);
        pa_threaded_mainloop_unlock(m_mainLoop);
        m_context = 0;
    }

    if (m_mainLoop) {
        pa_threaded_mainloop_stop(m_mainLoop);
        pa_threaded_mainloop_free(m_mainLoop);
        m_mainLoop = 0;
    }
}


PulseSink *PulseInterface::default_sink() {
       QMutexLocker l(&m_data_mutex);
       return m_default_sink;
}


static void cb_subscribe_success(pa_context *c, int success, void *userdata)
{
    Q_UNUSED(c);
    PulseInterface *pulse_iface = reinterpret_cast<PulseInterface*>(userdata);
    if (!success) {
        qWarning() << "Failed to subscribe. Live list updates impossible.";
    }
    pa_threaded_mainloop_signal(pulse_iface->mainloop(), 0);
}


void PulseInterface::subscribe()
{
    pa_threaded_mainloop_lock(m_mainLoop);
    pa_context_set_subscribe_callback(m_context, cb_subscribe, this);
    _COMPLETE_PA_OP(pa_context_subscribe(m_context, (pa_subscription_mask_t) (
                                             PA_SUBSCRIPTION_MASK_SINK
                                             | PA_SUBSCRIPTION_MASK_SINK_INPUT
                                             | PA_SUBSCRIPTION_MASK_SERVER),
                                         cb_subscribe_success, this))
}


static void cb_set_default_sink_success(pa_context *c, int success, void *userdata)
{
    Q_UNUSED(c);
    PulseInterface *pulse_iface = reinterpret_cast<PulseInterface*>(userdata);
    if (!success) {
        qWarning() << "Failed to set default sink.";
    }
    pa_threaded_mainloop_signal(pulse_iface->mainloop(), 0);
}


void PulseInterface::set_default_sink(PulseSink *sink) {
    QByteArray sink_name(sink->name().toUtf8());
    pa_threaded_mainloop_lock(m_mainLoop);
    _COMPLETE_PA_OP(pa_context_set_default_sink(m_context, sink_name.data(),
                                                cb_set_default_sink_success, this));
}


void PulseInterface::get_server_info()
{
    pa_threaded_mainloop_lock(m_mainLoop);
    _COMPLETE_PA_OP(pa_context_get_server_info(m_context, cb_server_info, this));
}

void PulseInterface::get_sinks()
{
    pa_threaded_mainloop_lock(m_mainLoop);
    _COMPLETE_PA_OP(pa_context_get_sink_info_list(m_context, cb_sink_info, this));
}

void PulseInterface::get_streams()
{
    pa_threaded_mainloop_lock(m_mainLoop);
    _COMPLETE_PA_OP(pa_context_get_sink_input_info_list(m_context, cb_stream_info, this));
}


static void cb_load_module(pa_context *c, uint32_t idx, void *userdata) {
    Q_UNUSED(c);
    PulseInterface *pulse_iface = reinterpret_cast<PulseInterface*>(userdata);
    if (!idx)
        emit pulse_iface->runtime_error("Failed to load module.");
    pa_threaded_mainloop_signal(pulse_iface->mainloop(), 0);
}


void PulseInterface::add_tunnel_sink(QString host, QString sink)
{
    QString tunnel_opt = QString("server=%1 sink=%2").arg(host, sink);
    QByteArray tunnel_opt_bytes = tunnel_opt.toUtf8();

    pa_threaded_mainloop_lock(m_mainLoop);
    _COMPLETE_PA_OP(pa_context_load_module(m_context, "module-tunnel-sink", tunnel_opt_bytes.data(), cb_load_module, this));
}


static void cb_move_stream_success(pa_context *c, int success, void *userdata)
{
    Q_UNUSED(c);
    PulseInterface *pulse_iface = reinterpret_cast<PulseInterface*>(userdata);
    if (!success)
        emit pulse_iface->runtime_error("Failed to move stream.");
    pa_threaded_mainloop_signal(pulse_iface->mainloop(), 0);
}


void PulseInterface::move_stream(PulseStream const &stream, PulseSink const *sink)
{
    pa_threaded_mainloop_lock(m_mainLoop);
    _COMPLETE_PA_OP(pa_context_move_sink_input_by_index(
                        m_context, stream.index(), sink->index(), cb_move_stream_success, this));
}


static void cb_unload_module(pa_context *c, int success, void *userdata) {
    Q_UNUSED(c);
    PulseInterface *pulse_iface = reinterpret_cast<PulseInterface*>(userdata);
    if (!success) {
        QString err = QString("Failed to unload module: %1").arg(pa_strerror(pa_context_errno(c)));
        qWarning() << err;
        emit pulse_iface->runtime_error(err);
    }
    pa_threaded_mainloop_signal(pulse_iface->mainloop(), 0);
}


void PulseInterface::unload_sink(QObject *o)
{
    PulseSink const *sink = qobject_cast<PulseSink const *>(o);
    pa_threaded_mainloop_lock(m_mainLoop);
    _COMPLETE_PA_OP(pa_context_unload_module(m_context, sink->module_index(), cb_unload_module, this));
}


QObject *PulseInterface::instance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);
    return the_instance();
}

QT_END_NAMESPACE
