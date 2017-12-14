#include <QDebug>
#include <QMutexLocker>
#include <QCoreApplication>

#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "pulse_interface.h"

QT_BEGIN_NAMESPACE


#define _PA_OP(_pa_iface_op_func) \
    pa_operation *_pa_iface_op = _pa_iface_op_func; \
    while (pa_operation_get_state(_pa_iface_op) == PA_OPERATION_RUNNING) \
        pa_threaded_mainloop_wait(m_mainLoop); \
    pa_operation_unref(_pa_iface_op);

#define _PA_LOCKED(code) \
    pa_threaded_mainloop_lock(m_mainLoop); \
    code \
    pa_threaded_mainloop_unlock(m_mainLoop);



static void cb_server_info(pa_context *context, const pa_server_info *info, void *userdata)
{
    PulseInterface *pulseEngine = static_cast<PulseInterface*>(userdata);

    if (!info) {
        qWarning() << QString("Failed to get server information: %1").arg(
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
        qWarning() << QString("Failed to get sink information: %1").arg(
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
    PulseSink pulse_sink((sink));
    pulse_sink.moveToThread(QCoreApplication::instance()->thread());
    int idx = pulseEngine->m_sinks.find_pulse_index(pulse_sink.index());
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
        qWarning() << QString("Failed to get stream information: %1").arg(
                          pa_strerror(pa_context_errno(context)));
        return;
    }

    if (eol) {
        pa_threaded_mainloop_signal(pulseEngine->mainloop(), 0);
        if (!pulseEngine->m_stream_updates_deferred && pulseEngine->m_streams_changed) {
            emit pulseEngine->stream_list_changed();
            pulseEngine->m_streams_changed = false;
        }
        return;
    }

    Q_ASSERT(stream);
    QMutexLocker(&(pulseEngine->m_data_mutex));

    PulseStream pulse_stream((stream));
    pulse_stream.moveToThread(QCoreApplication::instance()->thread());
    int idx = pulseEngine->m_streams.find_pulse_index(pulse_stream.index());
    if (idx < 0) {
        pulseEngine->m_streams.append(pulse_stream);
        pulseEngine->m_streams_changed = true;
    }
    else {
        pulseEngine->m_streams.replace(idx, pulse_stream);
    }
}


static void cb_module_info(pa_context *context, const pa_module_info *module, int eol, void *userdata)
{
    PulseInterface *pulseEngine = reinterpret_cast<PulseInterface*>(userdata);

    if (eol < 0) {
        qWarning() << QString("Failed to get stream information: %1").arg(
                          pa_strerror(pa_context_errno(context)));
        return;
    }

    if (eol) {
        pa_threaded_mainloop_signal(pulseEngine->mainloop(), 0);
        if (pulseEngine->m_modules_changed) {
            emit pulseEngine->module_list_changed();
            pulseEngine->m_modules_changed = false;
        }
        return;
    }

    Q_ASSERT(module);
    QMutexLocker(&(pulseEngine->m_data_mutex));

    PulseModule pulse_module((module));
    pulse_module.moveToThread(QCoreApplication::instance()->thread());
    int idx = pulseEngine->m_modules.find_pulse_index(pulse_module.index());
    if (idx < 0) {
        pulseEngine->m_modules.append(pulse_module);
        pulseEngine->m_modules_changed = true;
    }
    else {
        pulseEngine->m_modules.replace(idx, pulse_module);
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
                if (!pulse_iface->m_stream_updates_deferred)
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
    case PA_SUBSCRIPTION_EVENT_MODULE:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            QMutexLocker(&(pulse_iface->m_data_mutex));
            int list_idx = pulse_iface->m_modules.find_pulse_index(pa_idx);
            if (list_idx >= 0) {
                pulse_iface->m_modules.removeAt(list_idx);
                emit pulse_iface->module_list_changed();
            }
        }
        else {
            pa_operation *operation = pa_context_get_module_info(pulse_iface->context(), pa_idx, cb_module_info, userdata);
            if (!operation)
                emit pulse_iface->runtime_error(QString("Error getting info for module #%1").arg(pa_idx));
            pa_operation_unref(operation);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SERVER:
        pa_operation *op = pa_context_get_server_info(pulse_iface->context(), cb_server_info, pulse_iface);
        if (!op)
            emit pulse_iface->runtime_error(QString("Error getting server info."));
        pa_operation_unref(op);
        break;
    }
}

PulseInterface *PulseInterface::m_instance;

PulseInterface *PulseInterface::instance()
{
    if (!m_instance) m_instance = new PulseInterface();
    return m_instance;
}


bool PulseInterface::defer_stream_list_updates(bool is_deferred)
{
    bool was_deferred = m_stream_updates_deferred;
    m_stream_updates_deferred = is_deferred;
    if (m_streams_changed && was_deferred && !is_deferred) {
        emit stream_list_changed();
        m_streams_changed = false;
    }
    return was_deferred;
}


PulseInterface::PulseInterface(QObject *parent)
    : QObject(parent)
    , m_default_sink(NULL)
    , m_data_mutex(QMutex::Recursive)
    , m_stream_updates_deferred(false)
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
        subscribe();
        get_streams();
        get_server_info();
        get_modules();
    }
}

void PulseInterface::deleteLater()
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


QList<int> PulseInterface::find_modules(QString module_name)
{
    QList<int> rv;
    for (QList<PulseModule>::const_iterator it = m_modules.cbegin(); it != m_modules.cend(); ++it) {
        if (it->name() == module_name)
            rv.append(it->index());
    }
    return rv;
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
    _PA_LOCKED(
                pa_context_set_subscribe_callback(m_context, cb_subscribe, this);
            _PA_OP(pa_context_subscribe(m_context, (pa_subscription_mask_t) (
                                                     PA_SUBSCRIPTION_MASK_SINK
                                                     | PA_SUBSCRIPTION_MASK_SINK_INPUT
                                                     | PA_SUBSCRIPTION_MASK_SERVER
                                                     | PA_SUBSCRIPTION_MASK_MODULE),
                                                 cb_subscribe_success, this))
            )
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
    _PA_LOCKED(
                _PA_OP(pa_context_set_default_sink(
                                    m_context, sink_name.data(),
                                    cb_set_default_sink_success, this));
            )
}


void PulseInterface::get_server_info()
{
    _PA_LOCKED(_PA_OP(pa_context_get_server_info(m_context, cb_server_info, this)));
}

void PulseInterface::get_sinks()
{
    _PA_LOCKED(_PA_OP(pa_context_get_sink_info_list(m_context, cb_sink_info, this)));
}

void PulseInterface::get_streams()
{
    _PA_LOCKED(_PA_OP(pa_context_get_sink_input_info_list(m_context, cb_stream_info, this)));
}

void PulseInterface::get_modules()
{
    _PA_LOCKED(_PA_OP(pa_context_get_module_info_list(m_context, cb_module_info, this)));
}


static void cb_load_module(pa_context *c, uint32_t idx, void *userdata) {
    Q_UNUSED(c);
    PulseInterface *pulse_iface = reinterpret_cast<PulseInterface*>(userdata);
    if (!idx)
        emit pulse_iface->runtime_error("Failed to load module.");
    pa_threaded_mainloop_signal(pulse_iface->mainloop(), 0);
}


void PulseInterface::load_module(QString name, QString args)
{
   QByteArray args_bytes = args.toUtf8();
   QByteArray name_bytes = name.toUtf8();

   _PA_LOCKED(_PA_OP(pa_context_load_module(m_context, name_bytes.data(), args_bytes.data(), cb_load_module, this)));
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
    _PA_LOCKED(_PA_OP(pa_context_move_sink_input_by_index(
                        m_context, stream.index(), sink->index(), cb_move_stream_success, this)));
}

static void cb_unmute_stream_success(pa_context *c, int success, void *userdata)
{
    Q_UNUSED(c);
    PulseInterface *pulse_iface = reinterpret_cast<PulseInterface*>(userdata);
    if (!success)
        emit pulse_iface->runtime_error("Failed to unmute stream.");
    pa_threaded_mainloop_signal(pulse_iface->mainloop(), 0);
}

void PulseInterface::unmute_stream(PulseStream const &stream)
{
    _PA_LOCKED(_PA_OP(pa_context_set_sink_input_mute(
                        m_context, stream.index(), 0, cb_unmute_stream_success, this)));
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


void PulseInterface::unload_module(QObject *module)
{
    PulseModule const *m = qobject_cast<PulseModule const *>(module);
    _PA_LOCKED(_PA_OP(pa_context_unload_module(m_context, m->index(), cb_unload_module, this)));
}


void PulseInterface::unload_module(int module_idx)
{
    _PA_LOCKED(_PA_OP(pa_context_unload_module(m_context, module_idx, cb_unload_module, this)));
}


void PulseInterface::unload_sink(QObject *o)
{
    PulseSink const *sink = qobject_cast<PulseSink const *>(o);
    _PA_LOCKED(_PA_OP(pa_context_unload_module(m_context, sink->module_index(), cb_unload_module, this)));
}


QT_END_NAMESPACE
