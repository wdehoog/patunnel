/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QDebug>
#include <QQmlEngine>
#include <QJSEngine>

#include "pulse_interface.h"
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

QT_BEGIN_NAMESPACE

PulseSink *PulseInterface::find_sink_by_name(char const *name) {
    for (QList<PulseSink>::iterator it = m_sinks.begin(); it != m_sinks.end(); ++it) {
        if (it->name() == name)
            return &(*it);
    }
    return NULL;
}

PulseStream *PulseInterface::find_stream_by_idx(u_int32_t idx) {
    for (QList<PulseStream>::iterator it = m_streams.begin(); it != m_streams.end(); ++it) {
        if (idx == it->index())
            return &(*it);
    }
    return NULL;
}

PulseSink *PulseInterface::find_sink_by_idx(unsigned int idx) {
    for (QList<PulseSink>::iterator it = m_sinks.begin(); it != m_sinks.end(); it++) {
        if (idx == it->index())
            return &(*it);
    }
    return NULL;
}


static void cb_server_info(pa_context *context, const pa_server_info *info, void *userdata)
{
#ifdef DEBUG_PULSE
    char ss[PA_SAMPLE_SPEC_SNPRINT_MAX], cm[PA_CHANNEL_MAP_SNPRINT_MAX];

    pa_sample_spec_snprint(ss, sizeof(ss), &info->sample_spec);
    pa_channel_map_snprint(cm, sizeof(cm), &info->channel_map);

    qDebug() << QString("User name: %1\n"
             "Host Name: %2\n"
             "Server Name: %3\n"
             "Server Version: %4\n"
             "Default Sample Specification: %5\n"
             "Default Channel Map: %6\n"
             "Default Sink: %7\n"
             "Default Source: %8\n").arg(
           info->user_name,
           info->host_name,
           info->server_name,
           info->server_version,
           ss,
           cm,
           info->default_sink_name,
           info->default_source_name);
#endif
    PulseInterface *pulseEngine = static_cast<PulseInterface*>(userdata);

    if (!info) {
        qWarning() << QString("Failed to get server information: %s").arg(
                          pa_strerror(pa_context_errno(context)));
    }
    else {
        pulseEngine->m_default_sink = pulseEngine->find_sink_by_name(
                                            info->default_sink_name);
        emit pulseEngine->default_sink_changed();
        qDebug() << "Default sink:" << info->default_sink_name;
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

    PulseSink pulse_sink(sink);
    int idx;
    if ((idx = pulseEngine->m_sinks.indexOf(pulse_sink)) < 0) {
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
        qWarning() << QString("Failed to get stream information: %s").arg(pa_strerror(pa_context_errno(context)));
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
    PulseStream pulse_stream(stream);
    int idx;
    if ((idx = pulseEngine->m_streams.indexOf(pulse_stream)) < 0) {
        pulseEngine->m_streams.append(pulse_stream);
        pulseEngine->m_streams_changed = true;
    }
    else {
        pulseEngine->m_streams.replace(idx, pulse_stream);
    }
}


static void cb_subscribe(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata)
{
    Q_UNUSED(c);
    PulseInterface *pulse_iface = reinterpret_cast<PulseInterface*>(userdata);
    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
    case PA_SUBSCRIPTION_EVENT_SINK:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            QList<PulseSink>::iterator it = pulse_iface->m_sinks.begin();
            while (it != pulse_iface->m_sinks.end()) {
                if (it->index() == idx)
                    it = pulse_iface->m_sinks.erase(it);
                else ++it;
            }
            emit pulse_iface->sink_list_changed();
        }
        else {
            pa_operation *operation = pa_context_get_sink_info_by_index(
                        pulse_iface->context(), idx, cb_sink_info, userdata);
            if (!operation)
                emit pulse_iface->runtime_error(QString("Error getting info for sink #%1").arg(idx));
            pa_operation_unref(operation);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            QList<PulseStream>::iterator it = pulse_iface->m_streams.begin();
            while (it != pulse_iface->m_streams.end()) {
                if (it->index() == idx)
                    it = pulse_iface->m_streams.erase(it);
                else ++it;
            }
            emit pulse_iface->stream_list_changed();
        }
        else {
            pa_operation *operation = pa_context_get_sink_input_info(pulse_iface->context(), idx, cb_stream_info, userdata);
            if (!operation)
                emit pulse_iface->runtime_error(QString("Error getting info for stream #%1").arg(idx));
            pa_operation_unref(operation);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SERVER:
        pulse_iface->server_info();
        break;
    }
}

Q_GLOBAL_STATIC(PulseInterface, the_instance)

PulseInterface::PulseInterface(QObject *parent)
    : QObject(parent)
    , m_default_sink(NULL)
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
        sinks();
        server_info();
        streams();
        subscribe();
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
    pa_operation *op = pa_context_subscribe(m_context, (pa_subscription_mask_t) (
                                                PA_SUBSCRIPTION_MASK_SINK
                                                | PA_SUBSCRIPTION_MASK_SINK_INPUT),
                                            cb_subscribe_success, this);
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
        pa_threaded_mainloop_wait(m_mainLoop);
    pa_operation_unref(op);
    pa_threaded_mainloop_unlock(m_mainLoop);
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
    pa_threaded_mainloop_lock(m_mainLoop);
    pa_operation *op = pa_context_set_default_sink(m_context, sink->name(), cb_set_default_sink_success, this);
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
        pa_threaded_mainloop_wait(m_mainLoop);
    pa_operation_unref(op);
    pa_threaded_mainloop_unlock(m_mainLoop);
}


void PulseInterface::server_info()
{
    pa_threaded_mainloop_lock(m_mainLoop);
    pa_operation *operation = pa_context_get_server_info(m_context, cb_server_info, this);
    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING)
        pa_threaded_mainloop_wait(m_mainLoop);
    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(m_mainLoop);
}

void PulseInterface::sinks()
{
    pa_threaded_mainloop_lock(m_mainLoop);
    pa_operation *operation = pa_context_get_sink_info_list(m_context, cb_sink_info, this);
    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING)
        pa_threaded_mainloop_wait(m_mainLoop);
    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(m_mainLoop);
}

void PulseInterface::streams()
{
    pa_threaded_mainloop_lock(m_mainLoop);
    pa_operation *op = pa_context_get_sink_input_info_list(m_context, cb_stream_info, this);
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
        pa_threaded_mainloop_wait(m_mainLoop);
    pa_operation_unref(op);
    pa_threaded_mainloop_unlock(m_mainLoop);
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
    pa_operation *op = pa_context_load_module(m_context, "module-tunnel-sink", tunnel_opt_bytes.data(), cb_load_module, this);
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
        pa_threaded_mainloop_wait(m_mainLoop);
    pa_operation_unref(op);
    pa_threaded_mainloop_unlock(m_mainLoop);
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
    pa_operation *op = pa_context_move_sink_input_by_index(m_context, stream.index(), sink->index(), cb_move_stream_success, this);
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
        pa_threaded_mainloop_wait(m_mainLoop);
    pa_operation_unref(op);
    pa_threaded_mainloop_unlock(m_mainLoop);
}

QObject *PulseInterface::instance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);
    return the_instance();
}

QT_END_NAMESPACE
