//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "grumpyuijs.h"
#include "../scrollbacksmanager.h"
#include "../messagebox.h"
#include "../inputbox.h"
#include "../scrollbackframe.h"
#include <libcore/configuration.h>
#include <libcore/eventhandler.h>
#include <libcore/core.h>
#include <libcore/scripting/scriptextension.h>

using namespace GrumpyIRC;

GrumpyUIJS::GrumpyUIJS(ScriptExtension *s) : GenericJSClass(s)
{

}

QHash<QString, QString> GrumpyUIJS::GetFunctions()
{
    QHash<QString, QString> fh;
    fh.insert("load_history", "(scrollback_id)");
    fh.insert("clear_history", "(scrollback_id)");
    fh.insert("message_box", "(id, title, text)");
    return fh;
}

bool GrumpyUIJS::load_history(unsigned int scrollback_id, QString text)
{
    ScrollbackFrame *win = ScrollbacksManager::Global->GetWindowFromID(scrollback_id);
    if (!win)
    {
        GRUMPY_ERROR(this->script->GetName() + ": load_history(window, text): unknown frame");
        return false;
    }
    QList<QString> input = text.split("\n");
    win->GetInputBox()->LoadHistory(input);
    return true;
}

bool GrumpyUIJS::clear_history(unsigned int scrollback_id)
{
    ScrollbackFrame *win = ScrollbacksManager::Global->GetWindowFromID(scrollback_id);
    if (!win)
    {
        GRUMPY_ERROR(this->script->GetName() + ": clear_history(window): unknown frame");
        return false;
    }
    win->GetInputBox()->ClearHistory();
    return true;
}

bool GrumpyUIJS::message_box(QString id, QString title, QString text)
{
    MessageBox::Display(id, title, text);
    return true;
}
