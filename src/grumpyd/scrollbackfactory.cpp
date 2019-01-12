//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include "virtualscrollback.h"
#include "scrollbackfactory.h"

using namespace GrumpyIRC;

Scrollback *ScrollbackFactory::NewScrollback(Scrollback *parent, const QString &name, ScrollbackType type)
{
    VirtualScrollback *vs = new VirtualScrollback(type, parent);
    vs->SetTarget(name);
    return vs;
}

