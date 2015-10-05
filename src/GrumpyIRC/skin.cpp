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
}

QPalette GrumpyIRC::Skin::Palette()
{
    QPalette px;
    px.setColor(QPalette::ColorRole::Text, this->TextColor);
    px.setColor(QPalette::ColorRole::Base, this->BackgroundColor);
    return px;
}
