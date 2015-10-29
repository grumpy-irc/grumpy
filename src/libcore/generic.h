//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef GENERIC_H
#define GENERIC_H

#include <QString>
#include "libcore_global.h"
#include <QList>

namespace GrumpyIRC
{
    class Scrollback;

    namespace Generic
    {
        LIBCORESHARED_EXPORT bool String2Bool(QString string);
        LIBCORESHARED_EXPORT QString Bool2String(bool boolean);
        LIBCORESHARED_EXPORT QStringList Trim(QStringList list);
        LIBCORESHARED_EXPORT bool IsGrumpy(Scrollback *window);
    }
}


#endif // GENERIC_H
