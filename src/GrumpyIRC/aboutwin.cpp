//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "aboutwin.h"
#include "ui_aboutwin.h"

using namespace GrumpyIRC;

AboutWin::AboutWin(QWidget *parent) : QDialog(parent), ui(new Ui::AboutWin)
{
    this->ui->setupUi(this);
    this->ui->label_3->setText(QString("Version: ") + GRUMPY_VERSION_STRING\
                               "\n\n"\
                               "This program is licensed under GNU Lesser GPL v3.\n\n"\
                               "Copyright 2015 - 2017, Petr Bena");
    //this->ui->label->setPixmap(QPixmap(":/icons/img/exclamation-mark-s.png"));
}

AboutWin::~AboutWin()
{
    delete this->ui;
}
