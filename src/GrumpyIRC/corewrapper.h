//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef COREWRAPPER_H
#define COREWRAPPER_H

#ifdef GCFG
#undef GCFG
#endif
#define GCFG CoreWrapper::GrumpyCore->GetConfiguration()
// This macro expands to nickname as a QString taken from ini file
#define CONFIG_NICK GCFG->GetValueAsString("nick", "GrumpyUser")
#define SET_CONFIG_NICK(nick) GCFG->SetValue("nick", QVariant(nick))

namespace GrumpyIRC
{
    class Core;

    class CoreWrapper
    {
        public:
            static Core *GrumpyCore;
    };
}

#endif // COREWRAPPER_H
