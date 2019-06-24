//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef GRUMPYUIJS_H
#define GRUMPYUIJS_H

#include <libcore/scripting/genericjsclass.h>
#include <QJSEngine>

namespace GrumpyIRC
{
    class GrumpyUIJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            GrumpyUIJS(ScriptExtension *s);
            QHash<QString, QString> GetFunctions() override;
            Q_INVOKABLE bool load_history(unsigned int scrollback_id, QString text);
            Q_INVOKABLE bool clear_history(unsigned int scrollback_id);
            Q_INVOKABLE bool message_box(QString id, QString title, QString text);
    };
}

#endif // GRUMPYUIJS_H
