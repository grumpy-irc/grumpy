//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef SCROLLBACKJS_H
#define SCROLLBACKJS_H

#include "genericjsclass.h"
#include <QList>
#include <QJSEngine>

namespace GrumpyIRC
{
    class ScrollbackJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            ScrollbackJS(ScriptExtension *s);
            QHash<QString, QString> GetFunctions();
            Q_INVOKABLE bool write(unsigned int scrollback_id, QString text);
            Q_INVOKABLE bool has_network(unsigned int scrollback_id);
            Q_INVOKABLE bool has_network_session(unsigned int scrollback_id);
            Q_INVOKABLE QString get_type(unsigned int scrollback_id);
            Q_INVOKABLE QJSValue get_target(unsigned int scrollback_id);
            Q_INVOKABLE QJSValue create(unsigned int parent_id, QString name);
            Q_INVOKABLE bool remove(unsigned int scrollback_id);
            Q_INVOKABLE QList<int> list();
    };
}

#endif // SCROLLBACKJS_H
