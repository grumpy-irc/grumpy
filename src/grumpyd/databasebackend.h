//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef DATABASEBACKEND_H
#define DATABASEBACKEND_H

#include "../libcore/definitions.h"

namespace GrumpyIRC
{
    class DatabaseBackend
    {
        public:
            DatabaseBackend();
            virtual ~DatabaseBackend();
            virtual void LoadRoles()=0;
            virtual void LoadUsers()=0;
            virtual QHash<QString, QVariant> GetConfiguration(user_id_t user)=0;
            virtual void SetConfiguration(user_id_t user, QHash<QString, QVariant> data)=0;
    };
}

#endif // DATABASEBACKEND_H
