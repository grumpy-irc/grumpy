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

using namespace GrumpyIRC;

GrumpydScript::GrumpydScript() : ScriptExtension()
{

}

QString GrumpydScript::GetContext()
{
    return "grumpyd";
}

unsigned int GrumpydScript::GetContextID()
{
    return GRUMPY_SCRIPT_CONTEXT_GRUMPY_DAEMON;
}

int GrumpydScript::GetHookID(QString hook)
{
    return ScriptExtension::GetHookID(hook);
}

void GrumpydScript::registerFunctions()
{
    ScriptExtension::registerFunctions();
}

void GrumpydScript::registerClasses()
{

}
