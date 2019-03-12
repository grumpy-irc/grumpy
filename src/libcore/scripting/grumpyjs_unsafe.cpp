//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2019

#include <QDir>
#include <QFile>
#include "grumpyjs_unsafe.h"
#include "scriptextension.h"
#include "../scrollback.h"
#include "../configuration.h"
#include "../core.h"
#include "../eventhandler.h"
#include "../commandprocessor.h"

using namespace GrumpyIRC;

GrumpyJS_Unsafe::GrumpyJS_Unsafe(ScriptExtension *s) : GenericJSClass(s)
{

}

QHash<QString, QString> GrumpyJS_Unsafe::GetFunctions()
{
    QHash<QString, QString> fh;
    fh.insert("process", "(window_id, text): !unsafe! sends input to command processor, esentially same as entering text to input box in program");
    fh.insert("file_exists", "(path): returns true if file exists");
    return fh;
}

QJSValue GrumpyJS_Unsafe::process(unsigned int window_id, QString command)
{
    Scrollback *w = Scrollback::GetScrollbackByID(window_id);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": process(window_id, command): unknown scrollback");
        return QJSValue(false);
    }

    return QJSValue(Core::GrumpyCore->GetCommandProcessor()->ProcessText(command, w));
}

bool GrumpyJS_Unsafe::file_exists(QString path)
{
    return QFile::exists(path);
}
