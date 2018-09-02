//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef GRUMPYJS_H
#define GRUMPYJS_H

#include "genericjsclass.h"
#include <QVariant>
#include <QHash>
#include <QObject>
#include <QDateTime>
#include <QString>
#include <QJSEngine>

namespace GrumpyIRC
{
    class ScriptCommand;
    class GrumpyJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            GrumpyJS(ScriptExtension *s);
            ~GrumpyJS();
            QHash<QString, QString> GetFunctions();
            Q_INVOKABLE QString get_function_help(QString function_name);
            Q_INVOKABLE QList<QString> get_function_list();
            Q_INVOKABLE void log(QString text);
            Q_INVOKABLE void error_log(QString text);
            Q_INVOKABLE void debug_log(QString text, int verbosity = 1);
            Q_INVOKABLE bool register_hook(QString hook, QString function_name);
            Q_INVOKABLE void unregister_hook(QString hook);
            Q_INVOKABLE bool is_unsafe();
            Q_INVOKABLE bool register_cmd(QString name, QString fc);
            Q_INVOKABLE QList<QString> get_hook_list();
            Q_INVOKABLE bool has_function(QString f);
            Q_INVOKABLE QString get_context();
            Q_INVOKABLE int get_context_id();
            Q_INVOKABLE bool set_cfg(QString key, QJSValue value);
            Q_INVOKABLE QVariant get_cfg(QString key);
            Q_INVOKABLE QJSValue get_version();
        private:
            QList<ScriptCommand*> scriptCmds;
    };
}

#endif // GRUMPYJS_H
