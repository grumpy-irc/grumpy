//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef USERCONFIG_H
#define USERCONFIG_H

#include "../libcore/definitions.h"

#include "../libcore/configuration.h"

namespace GrumpyIRC
{
    class UserConf : public Configuration
    {
        public:
            UserConf(user_id_t user);
            void Save();
            void Load();
            user_id_t User;
    };
}

#endif // USERCONFIG_H
