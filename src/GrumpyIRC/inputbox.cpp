//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "../libcore/commandprocessor.h"
#include "corewrapper.h"
#include "../libcore/core.h"
#include "inputbox.h"
#include "ui_inputbox.h"

using namespace GrumpyIRC;

InputBox::InputBox(QWidget *parent) : QFrame(parent), ui(new Ui::InputBox)
{
    this->ui->setupUi(this);
}

InputBox::~InputBox()
{
    delete this->ui;
}

void GrumpyIRC::InputBox::on_lineEdit_returnPressed()
{
    CoreWrapper::GrumpyCore->GetCommandProcessor()->ProcessText(this->ui->lineEdit->text());
    this->ui->lineEdit->setText("");
}
