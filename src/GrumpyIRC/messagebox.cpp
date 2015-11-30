//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "messagebox.h"
#include "ui_messagebox.h"

using namespace GrumpyIRC;

MessageBox::MessageBox(QString id, QString title, QString message, QWidget *parent) : QDialog(parent), ui(new Ui::MessageBox)
{
    this->ui->setupUi(this);
    this->_id = id;
    this->_wt = title;
    this->_message = message;
}

MessageBoxResponse MessageBox::Exec(MessageBoxType type)
{
    this->ui->pushButton->setVisible(false);
    this->ui->pushButton_2->setVisible(false);
    this->ui->pushButton_3->setVisible(false);
    this->ui->pushButton_4->setVisible(false);
    switch (type)
    {
        case MessageBoxType_Normal:
            this->ui->pushButton->setVisible(true);
            break;
        case MessageBoxType_QuestionCancel:
            this->ui->pushButton_4->setVisible(true);
        case MessageBoxType_Question:
            this->ui->pushButton_2->setVisible(true);
            this->ui->pushButton_3->setVisible(true);
            break;
    }
    this->result = MessageBoxResponse_Cancel;
    this->exec();

    return this->result;
}

MessageBox::~MessageBox()
{
    delete this->ui;
}
