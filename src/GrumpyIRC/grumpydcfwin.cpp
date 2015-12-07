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
#include "grumpyconf.h"
#include "grumpydcfwin.h"
#include "ui_grumpydcfwin.h"

using namespace GrumpyIRC;

GrumpydCfWin::GrumpydCfWin(GrumpydSession *session, QWidget *parent) : QDialog(parent), ui(new Ui::GrumpydCfWin)
{
    ui->setupUi(this);
    this->GrumpySession = session;
    this->ui->checkBox->setChecked(this->getBool("offline_ms_bool", true));
    this->ui->plainTextEdit->setPlainText(this->getString("session_on_conn_raw", "AWAY"));
    this->ui->plainTextEdit_2->setPlainText(this->getString("session_on_disc_raw", "AWAY :" + CONF->GetDefaultAwayReason()));
    this->ui->lineEdit->setText(this->getString("offline_ms_text", this->ui->lineEdit->text()));
}

GrumpydCfWin::~GrumpydCfWin()
{
    delete ui;
}

void GrumpyIRC::GrumpydCfWin::on_buttonBox_accepted()
{
    this->set("offline_ms_bool", QVariant(this->ui->checkBox->isChecked()));
    this->set("offline_ms_text", QVariant(this->ui->lineEdit->text()));
    this->set("default_nick", this->ui->lineEdit_3->text());
    this->set("default_ident", this->ui->lineEdit_4->text());
    this->set("session_on_conn_raw", this->ui->plainTextEdit->toPlainText());
    this->set("session_on_disc_raw", this->ui->plainTextEdit_2->toPlainText());
    QHash<QString, QVariant> hash;
    hash.insert("merge", this->GrumpySession->Preferences);
    //this->set("away_msg", QVariant(this->ui->lineEdit_2->text()));
    this->GrumpySession->SendProtocolCommand(GP_CMD_OPTIONS, hash);
}

template <typename T>
void GrumpydCfWin::set(QString key, T value)
{
    if (this->GrumpySession->Preferences.contains(key))
        this->GrumpySession->Preferences[key] = value;
    else
        this->GrumpySession->Preferences.insert(key, value);
}

QString GrumpydCfWin::getString(QString key, QString missing)
{
    if (this->GrumpySession->Preferences.contains(key))
        return this->GrumpySession->Preferences[key].toString();
    return missing;
}

bool GrumpydCfWin::getBool(QString key, bool default_bool)
{
    if (this->GrumpySession->Preferences.contains(key))
        return this->GrumpySession->Preferences[key].toBool();
    return default_bool;
}
