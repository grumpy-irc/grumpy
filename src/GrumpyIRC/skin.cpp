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

Skin *GrumpyIRC::Skin::GetDefault()
{
	if (Skin::Default == NULL)
		Skin::Default = new Skin();
	return Skin::Default;
}

Skin::Skin()
{
    this->BackgroundColor = QColor(0, 0, 0);
    this->HighligtedColor = QColor(250, 184, 30);
    this->Error = QColor(255, 102, 102);
    this->Warning = QColor(255, 162, 162);
    this->SystemColor = QColor(71, 245, 92);
    this->TextColor = QColor(255, 255, 255);
#ifdef GRUMPY_WIN
    if (QFontDatabase().families().contains("Consolas"))
        this->TextFont = QFont("Consolas");
    else
        this->TextFont = QFont("Courier New");
#else
    this->TextFont = QFont("Monospace");
#endif
    this->TextFont.setPixelSize(13);
    this->SystemInfo = QColor(240, 250, 102);
    this->Unread = QColor(240, 250, 102);
    this->ModeColors.insert('v', QColor(244, 254, 10));
    this->ModeColors.insert('h', QColor(212, 250, 145));
    this->ModeColors.insert('o', QColor(92, 247, 14));
    this->ModeColors.insert('a', QColor(255, 156, 190));
    this->ModeColors.insert('q', QColor(255, 206, 156));
}

QPalette GrumpyIRC::Skin::Palette()
{
    QPalette px;
    px.setColor(QPalette::Text, this->TextColor);
    px.setColor(QPalette::Base, this->BackgroundColor);
    return px;
}
