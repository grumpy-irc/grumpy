//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2021

#include "favoriteswin.h"
#include "ui_favoriteswin.h"

using namespace GrumpyIRC;

FavoritesWin::FavoritesWin(QWidget *parent) :  QDialog(parent), ui(new Ui::FavoritesWin)
{
    this->ui->setupUi(this);
}

FavoritesWin::~FavoritesWin()
{
    delete this->ui;
}

