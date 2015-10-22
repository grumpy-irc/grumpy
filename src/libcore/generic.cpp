//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "generic.h"
#include "networksession.h"
#include "scrollback.h"

using namespace GrumpyIRC;

bool Generic::String2Bool(QString string)
{
    if (string.toLower() == "true")
        return true;
    return false;
}

QString Generic::Bool2String(bool boolean)
{
    if (boolean)
        return "true";
    return "false";
}

bool Generic::IsGrumpy(Scrollback *window)
{
    if (!window)
        return false;
    if (window->GetSession())
    {
        return window->GetSession()->GetType() == SessionType_Grumpyd;
    }
    return false;
}
