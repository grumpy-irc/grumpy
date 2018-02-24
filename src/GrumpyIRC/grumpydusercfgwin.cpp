//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "grumpydusercfgwin.h"
#include "messagebox.h"
#include "ui_grumpydusercfgwin.h"
#include <QMessageBox>
#include "../libcore/generic.h"
#include "../libcore/grumpydsession.h"

using namespace GrumpyIRC;

GrumpydUserCfgWin::GrumpydUserCfgWin(GrumpydSession *session, QWidget *parent) : QDialog(parent), ui(new Ui::GrumpydUserCfgWin)
{
    this->ui->setupUi(this);
    this->Session = session;
    this->setAttribute(Qt::WA_DeleteOnClose);
    foreach (QString role, session->GetRoles())
        this->ui->comboBox->addItem(role);
    this->ui->comboBox->addItem("Default");

    this->ui->comboBox->setCurrentIndex(this->ui->comboBox->count() - 1);
}

GrumpydUserCfgWin::~GrumpydUserCfgWin()
{
    delete this->ui;
}

void GrumpydUserCfgWin::UpdateUser(user_id_t id, QString name, QString role)
{
    this->user_id = (int)id;
    this->original_role = role;
    this->ui->Name->setEnabled(false);
    this->ui->Name->setText(name);
    this->ui->comboBox->setCurrentIndex(this->Session->GetRoles().indexOf(role));
}

void GrumpydUserCfgWin::on_Update_clicked()
{
    if (this->ui->Name->text().isEmpty())
    {
        this->Error("Username is empty");
        return;
    }

    if (this->ui->Pass2->text() != this->ui->Pass1->text())
    {
        this->Error("Passwords do not match");
        return;
    }

    if (this->ui->comboBox->currentText().isEmpty())
    {
        this->Error("Invalid role");
        return;
    }

    QHash<QString, QVariant> params;

    if (this->user_id >= 0)
    {
        // Update

        return;
    }

    params.insert("username", this->ui->Name->text());
    params.insert("password", this->ui->Pass1->text());
    if (this->ui->comboBox->currentIndex() != this->ui->comboBox->count() - 1)
        params.insert("role", this->ui->comboBox->currentText());
    this->Session->SendProtocolCommand(GP_CMD_SYS_CREATE_USER, params);
    this->Session->SendProtocolCommand(GP_CMD_SYS_LIST_USER);
    this->close();
}

void GrumpydUserCfgWin::Error(QString what)
{
    MessageBox::Display("grumpyduser-fail", "Error", what, this);
}
