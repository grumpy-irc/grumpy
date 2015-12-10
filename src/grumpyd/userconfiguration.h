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

// Some values explained:

// maximum_bsize is maximum size of items in buffer of scrollback, if there are more items than maximum_bsize the oldest items are removed from it.
//               Because the items are stored in sqlite we don't really need to keep them in buffer so this value should be rather small.

namespace GrumpyIRC
{
    class UserConf : public Configuration
    {
        public:
            UserConf(user_id_t user);
            void Save();
            void Load();
            void SetHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();
            user_id_t User;
    };
}

#endif // USERCONFIG_H
