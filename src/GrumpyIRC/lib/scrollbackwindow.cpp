//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2020

#include "scrollbackwindow.h"
#include "scrollbackframe.h"
#include "ui_scrollbackwindow.h"

using namespace GrumpyIRC;

ScrollbackWindow::ScrollbackWindow(QString name, QWidget *parent) : QDialog(parent), ui(new Ui::ScrollbackWindow)
{
    this->ui->setupUi(this);
    this->sf = new ScrollbackFrame(nullptr, this, nullptr, true);
    this->sf->IsStandalone = true;
    this->ui->verticalLayout->addWidget(this->sf);
    this->setWindowTitle(name);
    this->sf->SetWindowName(name);
    this->setAttribute(Qt::WA_DeleteOnClose);
}

Scrollback *ScrollbackWindow::GetScrollback()
{
    return this->sf->GetScrollback();
}

ScrollbackWindow::~ScrollbackWindow()
{
    delete this->sf;
    delete this->ui;
}
