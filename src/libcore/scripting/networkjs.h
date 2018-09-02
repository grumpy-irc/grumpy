//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef NETWORKJS_H
#define NETWORKJS_H

#include "genericjsclass.h"
#include <QJSEngine>

namespace GrumpyIRC
{
    class NetworkJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            NetworkJS(ScriptExtension *s);
            QHash<QString, QString> GetFunctions();
            Q_INVOKABLE QJSValue get_nick(unsigned int scrollback_id);
            Q_INVOKABLE QJSValue get_ident(unsigned int scrollback_id);
            Q_INVOKABLE QJSValue get_host(unsigned int scrollback_id);
            Q_INVOKABLE QJSValue get_network_name(unsigned int scrollback_id);
            Q_INVOKABLE QJSValue get_server_host(unsigned int scrollback_id);
            Q_INVOKABLE bool send_raw(unsigned int scrollback_id, QString raw);
            Q_INVOKABLE bool send_message(unsigned int scrollback_id, QString target, QString message);
    };
}

#endif // NETWORKJS_H
