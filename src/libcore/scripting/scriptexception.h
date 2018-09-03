//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef SCRIPTEXCEPTION_H
#define SCRIPTEXCEPTION_H

#include "../exception.h"

namespace GrumpyIRC
{
    class ScriptExtension;
    class LIBCORESHARED_EXPORT ScriptException : public Exception
    {
        public:
            ScriptException(QString Message, QString function_id, ScriptExtension *extension);
            ~ScriptException();
    };
}

#endif // SCRIPTEXCEPTION_H
