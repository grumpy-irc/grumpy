//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SKIN_H
#define SKIN_H

#include "../libcore/definitions.h"

#include <QHash>
#include <QFont>
#include <QPalette>

namespace GrumpyIRC
{
    class Skin
    {
        public:
            static Skin *Default;
			static Skin *GetDefault();

            Skin();
            QPalette Palette();
            QFont TextFont;
            QColor TextColor;
            QColor UserListAwayColor;
            QColor HighligtedColor;
            QColor SystemInfo;
            QColor Error;
            QColor Warning;
            QColor SystemColor;
            QColor Unread;
            QColor BackgroundColor;
            QHash<char, QColor> ModeColors;
    };
}

#endif // SKIN_H
