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

using namespace GrumpyIRC;

Skin *Skin::Default = new Skin();

Skin::Skin()
{
    this->BackgroundColor = QColor(0, 0, 0);
    this->TextColor = QColor(255, 255, 255);
    this->ModeColors.insert('v', QColor(244, 254, 10));
    this->ModeColors.insert('h', QColor(212, 250, 145));
    this->ModeColors.insert('o', QColor(92, 247, 14));
    this->ModeColors.insert('a', QColor(255, 156, 190));
    this->ModeColors.insert('q', QColor(255, 206, 156));
}

QPalette GrumpyIRC::Skin::Palette()
{
    QPalette px;
    px.setColor(QPalette::ColorRole::Text, this->TextColor);
    px.setColor(QPalette::ColorRole::Base, this->BackgroundColor);
    return px;
}
