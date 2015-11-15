//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "../libcore/grumpydsession.h"
#include "grumpydcfwin.h"
#include "ui_grumpydcfwin.h"

using namespace GrumpyIRC;

GrumpydCfWin::GrumpydCfWin(QWidget *parent) : QDialog(parent), ui(new Ui::GrumpydCfWin)
{
    ui->setupUi(this);
    //this->GrumpySession = NULL
}

GrumpydCfWin::~GrumpydCfWin()
{
    delete ui;
}

void GrumpyIRC::GrumpydCfWin::on_buttonBox_accepted()
{
    QHash<QString, QVariant> results;
    results.insert("offline_ms_bool", QVariant(this->ui->checkBox->isChecked()));
    results.insert("offline_ms_text", QVariant(this->ui->lineEdit->text()));
    //results.insert("away", QVariant(false))
    results.insert("away_msg", QVariant(this->ui->lineEdit_2->text()));
    this->GrumpySession->SendProtocolCommand(GP_CMD_OPTIONS, results);
}
