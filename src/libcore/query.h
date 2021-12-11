//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef QUERY_H
#define QUERY_H

#include "libcore_global.h"
#include <QString>
#include "../libirc/libirc/serializableitem.h"

namespace GrumpyIRC
{
    class ScrollbackItem;

    //! This is unused work in progress
    //! query for matching text, used by searching feature which is to be implemented
    class LIBCORESHARED_EXPORT Query : public libirc::SerializableItem
    {
        public:
            Query();
            bool Matches(ScrollbackItem *message);
             ~Query() override;
            QString Pattern;
            bool CS;
            bool IsRegex;
    };
}

#endif // QUERY_H
