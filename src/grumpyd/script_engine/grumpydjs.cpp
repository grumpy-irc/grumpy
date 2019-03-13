//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2019

#include "grumpydjs.h"
#include "../user.h"
#include "../virtualscrollback.h"

using namespace GrumpyIRC;

GrumpydJS::GrumpydJS(ScriptExtension *s) : GenericJSClass (s)
{

}

QHash<QString, QString> GrumpydJS::GetFunctions()
{
    QHash<QString, QString> functions_help;

    return functions_help;
}
