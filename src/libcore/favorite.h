//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef FAVORITE_H
#define FAVORITE_H

#include "definitions.h"
#include "../libirc/libirc/serializableitem.h"
#include "libcore_global.h"

#include <QList>
#include <QString>

namespace GrumpyIRC
{
    class LIBCORESHARED_EXPORT Favorite : public libirc::SerializableItem
    {
        public:
            Favorite();


        private:
            QString name;
            QString host;
            QString nick;
            QString username;
            QString password;

    };
}

#endif // FAVORITE_H
