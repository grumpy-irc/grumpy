//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "userwidget.h"
#include "ui_userwidget.h"

using namespace GrumpyIRC;

QFrame *currentWidget = NULL;

UserWidget::UserWidget(QWidget *parent) : QDockWidget(parent), ui(new Ui::UserWidget)
{
    this->ui->setupUi(this);
}

UserWidget::~UserWidget()
{
    delete ui;
}

void UserWidget::SetFrame(QFrame *frame)
{
    if (currentWidget != NULL)
    {
        QLayoutItem *container = this->ui->verticalLayout_2->itemAt(0);
        this->ui->verticalLayout_2->removeItem(this->ui->verticalLayout_2->itemAt(0));
        this->ui->verticalLayout_2->removeWidget(currentWidget);
        currentWidget->hide();
        this->ui->verticalLayout_2->addWidget(frame);
        frame->show();
        delete container;
    }
    else
    {
        this->ui->verticalLayout_2->addWidget(frame);
    }
    currentWidget = frame;
}
