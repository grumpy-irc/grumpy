//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "definitions.h"
#include "libcore_global.h"

#ifndef RESOURCES_H
#define RESOURCES_H

namespace GrumpyIRC
{
    class LIBCORESHARED_EXPORT Resources
    {
        public:
            static QString GetSource(QString path);
        private:
            static void init();
            static bool isInit;
    };
}

#endif
