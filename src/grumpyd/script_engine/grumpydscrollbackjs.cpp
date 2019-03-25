//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2019

#include "grumpydscript.h"
#include "grumpydscrollbackjs.h"
#include "../user.h"
#include "../virtualscrollback.h"
#include "../../libcore/eventhandler.h"
#include "../../libcore/core.h"

using namespace GrumpyIRC;

GrumpydScrollbackJS::GrumpydScrollbackJS(ScriptExtension *s) : GenericJSClass (s)
{

}

QHash<QString, QString> GrumpydScrollbackJS::GetFunctions()
{
    QHash<QString, QString> functions_help;
    functions_help.insert("get_owner_id", "(scrollback)");
    functions_help.insert("get_owner_name", "(scrollback)");
    return functions_help;
}

int GrumpydScrollbackJS::get_owner_id(unsigned int scrollback)
{
    VirtualScrollback *w = (VirtualScrollback*)Scrollback::GetScrollbackByID(scrollback);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": has_network(scrollback_id): unknown scrollback");
        return -1;
    }
    return w->GetOwner()->GetID();
}

QString GrumpydScrollbackJS::get_owner_name(unsigned int scrollback)
{
    VirtualScrollback *w = (VirtualScrollback*)Scrollback::GetScrollbackByID(scrollback);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": has_network(scrollback_id): unknown scrollback");
        return "";
    }
    return w->GetOwner()->GetName();
}

