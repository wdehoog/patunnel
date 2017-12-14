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

#ifndef PULSE_INTERFACE_H
#define PULSE_INTERFACE_H

#include <QtCore/qbytearray.h>
#include <QQmlEngine>
#include <QJSEngine>
#include <QQmlListProperty>
#include <QMutex>
#include <QMutexLocker>

#include <pulse/pulseaudio.h>
#include "pulse_object_list.h"
#include "pulse_object.h"
#include "pulse_sink.h"
#include "pulse_stream.h"
#include "pulse_module.h"

QT_BEGIN_NAMESPACE

#define QQML_LIST_PROPERTY_AT(name, type, backing_member) \
    static type *name(QQmlListProperty<type> *list_prop, int index) { \
        PulseInterface *iface = qobject_cast<PulseInterface *>(list_prop->object); \
        if (iface) { \
            QMutexLocker l(&(iface->m_data_mutex)); \
            return new type(iface->backing_member.at(index)); \
        } \
        return NULL; \
    }

#define QQML_LIST_PROPERTY_COUNT(name, type, backing_member) \
    static int name(QQmlListProperty<type> *list_prop) { \
        PulseInterface *iface = qobject_cast<PulseInterface *>(list_prop->object); \
        if (iface) { \
            QMutexLocker l(&(iface->m_data_mutex)); \
            return iface->backing_member.count(); \
        } \
        return -1; \
    }


class PulseInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PulseSink *default_sink
               READ default_sink
               WRITE set_default_sink
               NOTIFY default_sink_changed)
    Q_PROPERTY(QQmlListProperty<PulseSink> sink_list
               READ sink_list
               NOTIFY sink_list_changed)
    Q_PROPERTY(QQmlListProperty<PulseStream> stream_list
               READ stream_list
               NOTIFY stream_list_changed)
    Q_PROPERTY(QQmlListProperty<PulseModule> module_list
               READ module_list
               NOTIFY module_list_changed)

public:
    pa_threaded_mainloop *mainloop() { return m_mainLoop; }
    pa_context *context() { return m_context; }

    void set_default_sink(PulseSink *sink);
    PulseSink *default_sink();

    QQmlListProperty<PulseSink> sink_list() {
        return QQmlListProperty<PulseSink>(
                    this,
                    NULL,
                    &PulseInterface::sinks_prop_count,
                    &PulseInterface::sinks_prop_at);
    }

    QQmlListProperty<PulseStream> stream_list() {
        return QQmlListProperty<PulseStream>(
                    this,
                    NULL,
                    &PulseInterface::streams_prop_count,
                    &PulseInterface::streams_prop_at);
    }

    QQmlListProperty<PulseModule> module_list() {
        return QQmlListProperty<PulseModule>(
                    this,
                    NULL,
                    &PulseInterface::modules_prop_count,
                    &PulseInterface::modules_prop_at);
    }

    PulseSink *m_default_sink;
    PulseObjectList<PulseStream> m_streams;
    PulseObjectList<PulseSink> m_sinks;
    PulseObjectList<PulseModule> m_modules;

    QMutex m_data_mutex;

    bool m_sinks_changed;
    bool m_streams_changed;
    bool m_modules_changed;

    bool m_stream_updates_deferred;

    void get_server_info();
    void get_sinks();
    void get_streams();
    void get_modules();
    void subscribe();

    void move_stream(PulseStream const &stream, PulseSink const *sink);
    void unmute_stream(PulseStream const &stream);

    Q_INVOKABLE
    void load_module(QString name, QString args);

    Q_INVOKABLE
    void unload_module(QObject *module);

    Q_INVOKABLE
    void unload_module(int module_idx);

    Q_INVOKABLE
    void unload_sink(QObject *sink);

    Q_INVOKABLE
    bool defer_stream_list_updates(bool b);

    Q_INVOKABLE
    QList<int> find_modules(QString module_name);

    static PulseInterface *instance();

public slots:
    void deleteLater();

signals:
    void runtime_error(QString description);
    void default_sink_changed();
    void sink_list_changed();
    void stream_list_changed();
    void module_list_changed();

private:
    PulseInterface(QObject *parent = 0);

    pa_mainloop_api *m_mainLoopApi;
    pa_threaded_mainloop *m_mainLoop;
    pa_context *m_context;

    static PulseInterface *m_instance;

    QQML_LIST_PROPERTY_AT(sinks_prop_at, PulseSink, m_sinks)
    QQML_LIST_PROPERTY_AT(streams_prop_at, PulseStream, m_streams)
    QQML_LIST_PROPERTY_AT(modules_prop_at, PulseModule, m_modules)

    QQML_LIST_PROPERTY_COUNT(streams_prop_count, PulseStream, m_streams)
    QQML_LIST_PROPERTY_COUNT(sinks_prop_count, PulseSink, m_sinks)
    QQML_LIST_PROPERTY_COUNT(modules_prop_count, PulseModule, m_modules)
};


QT_END_NAMESPACE

#endif /*PULSE_INTERFACE_H*/
