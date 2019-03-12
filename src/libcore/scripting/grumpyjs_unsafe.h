//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2019

#ifndef GRUMPYJS_UNSAFE_H
#define GRUMPYJS_UNSAFE_H

#include "genericjsclass.h"
#include <QVariant>
#include <QHash>
#include <QObject>
#include <QDateTime>
#include <QString>
#include <QJSEngine>

namespace GrumpyIRC
{
    class GrumpyJS_Unsafe : public GenericJSClass
    {
            Q_OBJECT
        public:
            GrumpyJS_Unsafe(ScriptExtension *s);
            QHash<QString, QString> GetFunctions();
            Q_INVOKABLE QJSValue process(unsigned int window_id, QString command);
            Q_INVOKABLE bool file_exists(QString path);
    };
}

#endif // GRUMPYJS_UNSAFE_H
