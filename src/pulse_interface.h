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

#ifndef QPULSEAUDIOENGINE_H
#define QPULSEAUDIOENGINE_H

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

QT_BEGIN_NAMESPACE

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

private:
    static PulseSink *sinks_prop_at(QQmlListProperty<PulseSink> *list_prop, int index) {
        PulseInterface *iface = qobject_cast<PulseInterface *>(list_prop->object);
        if (iface) {
            QMutexLocker l(&(iface->m_data_mutex));
            return new PulseSink(iface->m_sinks.at(index));
        }
        return NULL;
    }

    static PulseStream *streams_prop_at(QQmlListProperty<PulseStream> *list_prop, int index) {
        PulseInterface *iface = qobject_cast<PulseInterface *>(list_prop->object);
        if (iface) {
            QMutexLocker l(&(iface->m_data_mutex));
            return new PulseStream(iface->m_streams.at(index));
        }
        return NULL;
    }

    static int sinks_prop_count(QQmlListProperty<PulseSink> *list_prop) {
        PulseInterface *iface = qobject_cast<PulseInterface *>(list_prop->object);
        if (iface) {
            QMutexLocker l(&(iface->m_data_mutex));
            return iface->m_sinks.count();
        }
        return -1;
    }

    static int streams_prop_count(QQmlListProperty<PulseStream> *list_prop) {
        PulseInterface *iface = qobject_cast<PulseInterface *>(list_prop->object);
        if (iface) {
            QMutexLocker l(&(iface->m_data_mutex));
            return iface->m_streams.count();
        }
        return -1;
    }

public:
    PulseInterface(QObject *parent = 0);
    ~PulseInterface();

    static QObject *instance(QQmlEngine *engine = NULL, QJSEngine *scriptEngine = NULL);
    pa_threaded_mainloop *mainloop() { return m_mainLoop; }
    pa_context *context() { return m_context; }

    void set_default_sink(PulseSink *sink);
    PulseSink *default_sink();

    QQmlListProperty<PulseSink> sink_list() {
        return QQmlListProperty<PulseSink>(this, NULL, &PulseInterface::sinks_prop_count, &PulseInterface::sinks_prop_at);
    }

    QQmlListProperty<PulseStream> stream_list() {
        return QQmlListProperty<PulseStream>(this, NULL, &PulseInterface::streams_prop_count, &PulseInterface::streams_prop_at);
    }

    PulseSink *m_default_sink;
    PulseObjectList<PulseStream> m_streams;
    PulseObjectList<PulseSink> m_sinks;

    QMutex m_data_mutex;

    bool m_sinks_changed;
    bool m_streams_changed;

    void get_server_info();
    void get_sinks();
    void get_streams();
    void subscribe();

    void move_stream(PulseStream const &stream, PulseSink const *sink);

    Q_INVOKABLE
    void add_tunnel_sink(QString host, QString sink);

    Q_INVOKABLE
    void unload_sink(QObject *sink);

public slots:

signals:
    void runtime_error(QString description);
    void default_sink_changed();
    void sink_list_changed();
    void stream_list_changed();

private:

    pa_mainloop_api *m_mainLoopApi;
    pa_threaded_mainloop *m_mainLoop;
    pa_context *m_context;

};

QT_END_NAMESPACE

#endif
