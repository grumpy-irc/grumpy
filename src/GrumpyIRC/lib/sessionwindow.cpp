//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "sessionwindow.h"
#include "ui_sessionwindow.h"

using namespace GrumpyIRC;

SessionWindow::SessionWindow(QWidget *parent) : QDialog(parent), ui(new Ui::SessionWindow)
{
    this->ui->setupUi(this);
}

SessionWindow::~SessionWindow()
{
    delete this->ui;
}
