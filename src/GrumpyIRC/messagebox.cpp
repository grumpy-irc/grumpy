//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "../libcore/definitions.h"
#include <QStyle>
#include "messagebox.h"
#include "grumpyconf.h"
#include "ui_messagebox.h"

using namespace GrumpyIRC;

void MessageBox::Display(QString id, QString title, QString message, QWidget *parent)
{
    if (CONF->IsDisabledMessage(id))
        return;
    MessageBox mb(id, title, message, parent);
    mb.Exec(MessageBoxType_Normal);
}

MessageBoxResponse MessageBox::Question(QString id, QString title, QString message, QWidget *parent)
{
    if (CONF->IsDisabledMessage(id))
        return MessageBoxResponse_Yes;
    MessageBox mb(id, title, message, parent);
    return mb.Exec(MessageBoxType_Question);
}

MessageBox::MessageBox(QString id, QString title, QString message, QWidget *parent) : QDialog(parent), ui(new Ui::MessageBox)
{
    this->ui->setupUi(this);
    this->_id = id;
    this->_wt = title;
    this->_message = message;
    this->ui->label->setText(message);
    this->setWindowTitle(title);
}

static QPixmap standardIcon(MessageBoxType mt, QWidget *mb)
{
    QStyle *style = mb ? mb->style() : QApplication::style();
    int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, mb);
    QIcon tmpIcon;
    switch (mt) {
    case MessageBoxType_Normal:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, 0, mb);
        break;
    //case QMessageBox::Warning:
    //    tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, 0, mb);
    //    break;
    case MessageBoxType_Error:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, 0, mb);
        break;
    case MessageBoxType_Question:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mb);
    default:
        break;
    }
    if (!tmpIcon.isNull())
        return tmpIcon.pixmap(iconSize, iconSize);
    return QPixmap();
}

MessageBoxResponse MessageBox::Exec(MessageBoxType type)
{
    this->ui->pushCancel->setVisible(false);
    this->ui->pushNO->setVisible(false);
    this->ui->pushYES->setVisible(false);
    this->ui->pushOK->setVisible(false);
    this->ui->label_2->setPixmap(standardIcon(type, this));
    switch (type)
    {
        case MessageBoxType_Normal:
            //this->ui->label->setPixmap();
            this->ui->pushOK->setVisible(true);
            break;
        case MessageBoxType_Error:
            this->ui->pushOK->setVisible(true);
            break;
        case MessageBoxType_QuestionCancel:
            this->ui->pushCancel->setVisible(true);
        case MessageBoxType_Question:
            this->ui->pushNO->setVisible(true);
            this->ui->pushYES->setVisible(true);
            break;
    }
    this->result = MessageBoxResponse_Cancel;
    this->exec();
    if (this->ui->checkBox->isChecked())
        CONF->SetDisabledMessage(true, this->_id);

    return this->result;
}

MessageBox::~MessageBox()
{
    delete this->ui;
}

void GrumpyIRC::MessageBox::on_pushYES_clicked()
{
    this->result = MessageBoxResponse_Yes;
    this->close();
}

void GrumpyIRC::MessageBox::on_pushNO_clicked()
{
    this->result = MessageBoxResponse_No;
    this->close();
}

void GrumpyIRC::MessageBox::on_pushCancel_clicked()
{
    this->close();
}

void GrumpyIRC::MessageBox::on_pushOK_clicked()
{
    this->result = MessageBoxResponse_OK;
    this->close();
}
