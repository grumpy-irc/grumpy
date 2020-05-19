//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2020

#ifndef GREP_H
#define GREP_H

#include <QList>
#include <QString>
#include "scrollbackitem.h"

namespace GrumpyIRC
{
    class GREP
    {
        public:
            GREP(QString regex, bool case_sensitive = true);
            ~GREP();
            bool IsValid();
            QList<ScrollbackItem> Exec(QList<ScrollbackItem> buff);
            int Context = 3;

        private:
            bool caseSensitive = true;
            QString regex;
    };
}

#endif // GREP_H
