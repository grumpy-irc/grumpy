//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2019

#ifndef GRUMPYJS_H
#define GRUMPYJS_H

#include "genericjsclass.h"
#include <QVariant>
#include <QHash>
#include <QObject>
#include <QDateTime>
#include <QString>
#include <QJSEngine>
#include <QTimer>

namespace GrumpyIRC
{
    class ScriptCommand;
    class GrumpyJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            GrumpyJS(ScriptExtension *s);
            ~GrumpyJS() override;
            QHash<QString, QString> GetFunctions() override;
            Q_INVOKABLE QString get_function_help(const QString& function_name);
            Q_INVOKABLE QList<QString> get_function_list();
            Q_INVOKABLE void log(const QString& text);
            Q_INVOKABLE void error_log(const QString& text);
            Q_INVOKABLE void debug_log(const QString& text, int verbosity = 1);
            Q_INVOKABLE bool register_hook(const QString& hook, const QString& function_name);
            Q_INVOKABLE void unregister_hook(const QString& hook);
            Q_INVOKABLE bool is_unsafe();
            Q_INVOKABLE bool register_cmd(const QString& name, const QString& fc);
            Q_INVOKABLE QList<QString> get_hook_list();
            Q_INVOKABLE bool has_function(const QString& f);
            Q_INVOKABLE QString get_context();
            Q_INVOKABLE int get_context_id();
            Q_INVOKABLE bool set_cfg(const QString& key, const QJSValue& value);
            Q_INVOKABLE QVariant get_cfg(const QString& key);
            Q_INVOKABLE QJSValue get_version();
            // Misc
            Q_INVOKABLE QString dump_obj(const QJSValue& object, unsigned int indent = 0);
            Q_INVOKABLE QJSValue seconds_to_time_span(int seconds);
            // Timers
            Q_INVOKABLE unsigned int create_timer(int interval, const QString& function, bool start = true);
            Q_INVOKABLE bool destroy_timer(unsigned int timer);
            Q_INVOKABLE bool start_timer(unsigned int timer, int interval);
            Q_INVOKABLE bool stop_timer(unsigned int timer);
            // Time
            Q_INVOKABLE qint64 get_startup_time_unix();
            Q_INVOKABLE qint64 get_uptime();
            Q_INVOKABLE QDateTime get_startup_date_time();
            Q_INVOKABLE QString get_current_time_str();
            Q_INVOKABLE int get_current_time_posix();
        private slots:
            void OnTime();
        private:
            QList<ScriptCommand*> scriptCmds;
            unsigned int lastTimer = 0;
            QHash<unsigned int, QTimer*> timers;
            QHash<QTimer*, QString> timerFunctions;
    };
}

#endif // GRUMPYJS_H
