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
#include <QList>
#include <QFont>
#include <QPalette>
#include "../libirc/libirc/serializableitem.h"

namespace GrumpyIRC
{
    class Skin : public libirc::SerializableItem
    {
        public:
			static Skin *GetDefault();
            static void Clear();
            static Skin *GetCurrent();
            static QList<Skin*> SkinList;
            static Skin *Default;
            static Skin *Current;

            Skin(QHash<QString, QVariant> skin);
            Skin();
            Skin(Skin *forked);
            ~Skin();
            QPalette Palette();
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();
            bool IsDefault();
            QString Name;
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
        private:
            void setDefaults();
    };
}

#endif // SKIN_H
