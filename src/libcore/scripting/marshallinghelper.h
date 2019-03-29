//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2019

#ifndef MARSHALLINGHELPER_H
#define MARSHALLINGHELPER_H

#include "../definitions.h"
#include "../libcore_global.h"
#include <QJSEngine>

namespace libircclient
{
    class Parser;
}

namespace GrumpyIRC
{
    namespace MarshallingHelper
    {
        LIBCORESHARED_EXPORT QJSValue FromParser(libircclient::Parser *px, QJSEngine *engine);
    }
}

#endif // MARSHALLINGHELPER_H
