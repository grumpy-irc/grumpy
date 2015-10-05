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
#include "skin.h"
#include "keyfilter.h"
#include "ui_inputbox.h"
#include "scrollbackframe.h"

using namespace GrumpyIRC;

InputBox::InputBox(ScrollbackFrame *parent) : QFrame(parent), ui(new Ui::InputBox)
{
    this->ui->setupUi(this);
    this->ui->textEdit->setPalette(Skin::Default->Palette());
    this->ui->textEdit->installEventFilter(new KeyFilter(this));
    this->parent = parent;
}

InputBox::~InputBox()
{
    delete this->ui;
}

void InputBox::ProcessInput()
{
    CoreWrapper::GrumpyCore->GetCommandProcessor()->ProcessText(this->ui->textEdit->toPlainText(), this->parent->GetScrollback());
    this->ui->textEdit->setText("");
}

void InputBox::Focus()
{
    this->ui->textEdit->setFocus();
}
