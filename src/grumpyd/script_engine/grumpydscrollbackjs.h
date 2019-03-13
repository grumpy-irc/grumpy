//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2019

#ifndef GRUMPYDSCROLLBACKJS_H
#define GRUMPYDSCROLLBACKJS_H

#include "../../libcore/scripting/genericjsclass.h"
#include <QHash>

namespace GrumpyIRC
{
    class GrumpydScrollbackJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            GrumpydScrollbackJS(ScriptExtension *s);
            QHash<QString, QString> GetFunctions();
            Q_INVOKABLE int get_owner(unsigned int scrollback);
    };
}

#endif // GRUMPYDSCROLLBACKJS_H
