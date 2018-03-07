//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "skin.h"
#include <QFontDatabase>
#include <QStringList>

using namespace GrumpyIRC;

Skin *Skin::Default = NULL;
Skin *Skin::Current = NULL;
QList<Skin*> Skin::SkinList;

Skin *Skin::GetDefault()
{
    if (Skin::Default == NULL)
        Skin::Default = new Skin();
    return Skin::Default;
}

void Skin::Clear()
{
    int s = 0;
    while (s < SkinList.count())
    {
        if (SkinList[s]->IsDefault())
            s++;
        else
            delete SkinList[s];
    }
}

Skin *Skin::GetCurrent()
{
    if (Skin::Current == NULL)
        Skin::Current = Skin::GetDefault();

    return Skin::Current;
}

Skin::Skin(QHash<QString, QVariant> skin)
{
    SkinList.append(this);
    this->setDefaults();
    this->LoadHash(skin);
}

Skin::Skin()
{
    SkinList.append(this);
    this->setDefaults();
}

Skin::~Skin()
{
    this->SkinList.removeAll(this);
}

QPalette GrumpyIRC::Skin::Palette()
{
    QPalette px;
    px.setColor(QPalette::Text, this->TextColor);
    px.setColor(QPalette::Base, this->BackgroundColor);
    return px;
}

void Skin::SetSize(int font_size)
{
    // Update the size
    this->TextSize = font_size;
    this->TextFont.setPixelSize(this->TextSize);
}

void Skin::LoadHash(QHash<QString, QVariant> hash)
{
    UNSERIALIZE_STRING(FontFamily);
    UNSERIALIZE_INT(TextSize);
    this->TextFont = QFont(this->FontFamily);
    this->TextFont.setPixelSize(this->TextSize);
    UNSERIALIZE_STRING(Name);
    UNSERIALIZE_COLOR(BackgroundColor);
    UNSERIALIZE_COLOR(Error);
    UNSERIALIZE_COLOR(HighligtedColor);
    UNSERIALIZE_COLOR(SystemColor);
    UNSERIALIZE_COLOR(SystemInfo);
    UNSERIALIZE_COLOR(TextColor);
    UNSERIALIZE_COLOR(Unread);
    UNSERIALIZE_COLOR(Timestamp);
    UNSERIALIZE_COLOR(UserColor);
    UNSERIALIZE_COLOR(LinkColor);
}

QHash<QString, QVariant> Skin::ToHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(Name);
    SERIALIZE(TextSize);
    SERIALIZE(FontFamily);
    SERIALIZE_COLOR(BackgroundColor);
    SERIALIZE_COLOR(Error);
    SERIALIZE_COLOR(HighligtedColor);
    //SERIALIZE(ModeColors);
    SERIALIZE_COLOR(SystemColor);
    SERIALIZE_COLOR(SystemInfo);
    SERIALIZE_COLOR(TextColor);
    SERIALIZE_COLOR(Unread);
    SERIALIZE_COLOR(Timestamp);
    SERIALIZE_COLOR(LinkColor);
    SERIALIZE_COLOR(UserColor);
    return hash;
}

bool Skin::IsDefault()
{
    return GetDefault() == this;
}

void Skin::setDefaults()
{
    this->Name = "default";
    this->BackgroundColor = QColor(0, 0, 0);
    this->HighligtedColor = QColor(250, 184, 30);
    this->Error = QColor(255, 102, 102);
    this->Warning = QColor(255, 162, 162);
    this->SystemColor = QColor(71, 245, 92);
    this->UserListAwayColor = QColor(180, 180, 180);
    this->TextColor = QColor(255, 255, 255);
    this->LinkColor = QColor("#94D7F2");
    this->Timestamp = QColor("#DCFCAC");
    this->UserColor = QColor("#B4D6FA");
    this->TextSize = 12;
#ifdef GRUMPY_WIN
    if (QFontDatabase().families().contains("Consolas"))
        this->FontFamily = "Consolas";
    else
        this->FontFamily = "Courier New";
#else
    this->FontFamily = "Monospace";
#endif
    this->TextFont = QFont(this->FontFamily);
    this->TextFont.setPixelSize(this->TextSize);
    this->SystemInfo = QColor(240, 250, 102);
    this->Unread = QColor(240, 250, 102);
    this->ModeColors.insert('v', QColor(244, 254, 10));
    this->ModeColors.insert('h', QColor(212, 250, 145));
    this->ModeColors.insert('o', QColor(92, 247, 14));
    this->ModeColors.insert('a', QColor(255, 156, 190));
    this->ModeColors.insert('q', QColor(255, 206, 156));
    this->Colors.insert(0,    QColor("#000000")); // White
    this->Colors.insert(1,    QColor("#FFFFFF")); // Black
    this->Colors.insert(2,    QColor("#97ADF7")); // Blue
    this->Colors.insert(3,    QColor("#A1F797")); // Green
    this->Colors.insert(4,    QColor("#FFC4CE")); // Light red
    this->Colors.insert(5,    QColor("#CCB472")); // Brown
    this->Colors.insert(6,    QColor("#F03CD5")); // Purple
    this->Colors.insert(7,    QColor("#F0B754")); // Orange
    this->Colors.insert(8,    QColor("#FCFA6A")); // Yellow
    this->Colors.insert(9,    QColor("#C3FAC5")); // Light green
    this->Colors.insert(10,   QColor("#C8FAF7")); // Cyan
    this->Colors.insert(11,   QColor("#DEFAF8")); // Light cyan
    this->Colors.insert(12,   QColor("#D2D3FA")); // Light blue
    this->Colors.insert(13,   QColor("#F7C3F1")); // Pink
    this->Colors.insert(14,   QColor("#C4C4C4")); // Grey
    this->Colors.insert(15,   QColor("#DADADA")); // Light grey
}

Skin::Skin(Skin *forked)
{
    SkinList.append(this);
    this->Name = forked->Name + " (fork)";
    this->BackgroundColor = forked->BackgroundColor;
    this->HighligtedColor = forked->HighligtedColor;
    this->Error = forked->Error;
    this->Warning = forked->Warning;
    this->SystemColor = forked->SystemColor;
    this->UserListAwayColor = forked->UserListAwayColor;
    this->TextColor = forked->TextColor;
    this->TextFont = forked->TextFont;
    this->SystemInfo = forked->SystemInfo;
    this->Unread = QColor(240, 250, 102);
    this->ModeColors.insert('v', QColor(244, 254, 10));
    this->ModeColors.insert('h', QColor(212, 250, 145));
    this->ModeColors.insert('o', QColor(92, 247, 14));
    this->ModeColors.insert('a', QColor(255, 156, 190));
    this->ModeColors.insert('q', QColor(255, 206, 156));
}
