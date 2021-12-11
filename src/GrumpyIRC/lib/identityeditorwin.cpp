//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2021

#include "identityeditorwin.h"
#include "ui_identityeditorwin.h"
#include "../../libcore/identity.h"

using namespace GrumpyIRC;

IdentityEditorWin::IdentityEditorWin(QWidget *parent) : QDialog(parent), ui(new Ui::IdentityEditorWin)
{
    this->ui->setupUi(this);
}

IdentityEditorWin::~IdentityEditorWin()
{
    delete this->ui;
}

void IdentityEditorWin::Load(int id)
{
    if (!Identity::Identities.contains(id))
        return;

    Identity *x = Identity::Identities[id];
    this->identID = id;
    this->ui->le_RealName->setText(x->RealName);
    this->ui->le_Nick->setText(x->Nick);
    this->ui->le_Ident->setText(x->Ident);
    this->ui->le_AwayMsg->setText(x->AwayMessage);
}

void GrumpyIRC::IdentityEditorWin::on_pbSave_clicked()
{
    if (this->identID < 0)
    {
        Identity *i = new Identity(this->ui->le_Nick->text(), this->ui->le_Ident->text(), this->ui->le_RealName->text(), this->ui->le_AwayMsg->text());
        Identity::Identities.insert(i->ID, i);
        this->close();
        return;
    }
    Identity *x = Identity::Identities[this->identID];
    x->Nick = this->ui->le_Nick->text();
    x->Ident = this->ui->le_Ident->text();
    x->RealName = this->ui->le_RealName->text();
    x->AwayMessage = this->ui->le_AwayMsg->text();
    this->close();
}

void GrumpyIRC::IdentityEditorWin::on_pbCancel_clicked()
{
    this->close();
}
