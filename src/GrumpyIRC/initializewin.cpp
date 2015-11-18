//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "initializewin.h"
#include "ui_initializewin.h"

using namespace GrumpyIRC;

InitializeWin::InitializeWin(QWidget *parent) : QDialog(parent), ui(new Ui::InitializeWin)
{
    this->ui->setupUi(this);
}

InitializeWin::~InitializeWin()
{
    delete this->ui;
}

void InitializeWin::Process()
{

}
