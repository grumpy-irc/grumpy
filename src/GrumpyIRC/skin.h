//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef SKIN_H
#define SKIN_H

#include "../libcore/definitions.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QFont>
#include <QPalette>
#include "../libirc/libirc/serializableitem.h"

#define SERIALIZE_COLOR(color)   hash.insert(#color, color.name())
#define UNSERIALIZE_COLOR(variable_name)       if (hash.contains(#variable_name)) { variable_name = QColor(hash[#variable_name].toString()); }

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
            void SetSize(int font_size);
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();
            bool IsDefault();
            QHash<unsigned int, QColor> Colors;
            QString Name;
            int TextSize;
            QString FontFamily;
            QFont TextFont;
            QColor TextColor;
            //! Color of username in the scrollback text, used by html
            QColor UserColor;
            QColor LinkColor;
            QColor UserListAwayColor;
            QColor HighligtedColor;
            QColor SystemInfo;
            QColor Error;
            QColor Warning;
            QColor SystemColor;
            QColor Timestamp;
            QColor Unread;
            QColor BackgroundColor;
            QString BackgroundImage;
            //! Changes opacity of background image relative to background color
            int BackgroundImageOpacity = 0;
            //! Changes opacity of whole GrumpyChat window
            int Opacity = 0;
            QHash<char, QColor> ModeColors;
        private:
            void setDefaults();
    };
}

#endif // SKIN_H
