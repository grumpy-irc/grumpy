//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "core.h"
#include "hooks.h"
#include "scrollback.h"
#include "extension.h"

using namespace GrumpyIRC;

void Hooks::OnScrollback_InsertText(Scrollback *scrollback, ScrollbackItem *item)
{

}

void Hooks::OnScrollback_Destroyed(Scrollback *scrollback)
{
    if (!Core::GrumpyCore)
        return;
    foreach (Extension *e, Core::GrumpyCore->GetExtensions())
    {
        e->Hook_OnScrollbackDestroyed(scrollback);
    }
}
