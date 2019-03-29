//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2019

#include "marshallinghelper.h"
#include "../../libirc/libircclient/parser.h"

using namespace GrumpyIRC;

QJSValue MarshallingHelper::FromParser(libircclient::Parser *px, QJSEngine *engine)
{
    QJSValue o = engine->newObject();
    o.setProperty("raw", QJSValue(px->GetRaw()));
    o.setProperty("text", QJSValue(px->GetText()));
    o.setProperty("numeric", QJSValue(px->GetNumeric()));
    o.setProperty("source_info", QJSValue(px->GetSourceInfo()));
    o.setProperty("parameter_line", QJSValue(px->GetParameterLine()));
    return o;
}

