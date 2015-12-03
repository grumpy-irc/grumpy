//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "preferenceswin.h"
#include "ui_preferenceswin.h"
#include "grumpyconf.h"

using namespace GrumpyIRC;

PreferencesWin::PreferencesWin(QWidget *parent) : QDialog(parent), ui(new Ui::PreferencesWin)
{
    this->ui->setupUi(this);
    this->ui->leIdent->setText(CONF->GetIdent());
    this->ui->leNick->setText(CONF->GetNick());
    this->ui->leNickFix->setText(CONF->GetAlterNick());
    this->ui->lineEdit->setText(CONF->GetQuitMessage());
    this->ui->lineEdit_2->setText(CONF->GetName());
    this->ui->lineEdit_3->setText(QString::number(CONF->GetSplitMaxSize()));
    this->ui->checkBoxSplitMs->setChecked(CONF->GetSplit());
}

PreferencesWin::~PreferencesWin()
{
    delete this->ui;
}

void GrumpyIRC::PreferencesWin::on_buttonBox_rejected()
{
    this->close();
}

void GrumpyIRC::PreferencesWin::on_buttonBox_accepted()
{
    CONF->SetAlterNick(this->ui->leNickFix->text());
    CONF->SetNick(this->ui->leNick->text());
    CONF->SetIdent(this->ui->leIdent->text());
    CONF->SetQuitMessage(this->ui->lineEdit->text());
    CONF->SetName(this->ui->lineEdit_2->text());
    CONF->SetSplitMaxSize(this->ui->lineEdit_3->text().toInt());
    CONF->SetSplit(this->ui->checkBoxSplitMs->isChecked());
    CONF->Save();
    this->close();
}
