//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2021

#include "networkwin.h"
#include <libcore/networkinfo.h>
#include <libcore/identity.h>
#include "ui_networkwin.h"

using namespace GrumpyIRC;

NetworkWin::NetworkWin(QWidget *parent) : QDialog(parent), ui(new Ui::NetworkWin)
{
    this->ui->setupUi(this);
    QList<int> identities = Identity::Identities.keys();
    int i = 1;
    this->ui->cb_IdentityNumber->addItem("Default");
    this->identityMap.insert(0, -1);
    foreach (int identity, identities)
    {
        this->identityMap.insert(i, identity);
        this->ui->cb_IdentityNumber->addItem(Identity::Identities[identity]->ToString());
        i++;
    }
}

NetworkWin::~NetworkWin()
{
    delete this->ui;
}

void NetworkWin::Load(int network_id)
{
     NetworkInfo *x = NetworkInfo::NetworksInfo[network_id];
     this->networkID = network_id;
     this->ui->le_Port->setText(QString::number(x->Port));
     this->ui->checkBox_SSL->setChecked(x->SSL);
     this->ui->le_HostName->setText(x->Hostname);
     this->ui->le_Name->setText(x->NetworkName);
     // find identity ID
     if (x->PreferredIdentity > -1)
     {
         int identity = -1;
         foreach (int id, this->identityMap.keys())
         {
             if (this->identityMap[id] == x->PreferredIdentity)
             {
                 identity = id;
                 break;
             }
         }
         if (identity > -1)
             this->ui->cb_IdentityNumber->setCurrentIndex(identity);
     }
}

void GrumpyIRC::NetworkWin::on_pushSave_clicked()
{
    if (this->networkID < 0)
    {
        NetworkInfo *network = new NetworkInfo(this->ui->le_Name->text(),
                                               this->ui->le_HostName->text(),
                                               this->ui->le_Port->text().toInt(),
                                               this->ui->checkBox_SSL->isChecked(),
                                               this->identityMap[this->ui->cb_IdentityNumber->currentIndex()]);
        NetworkInfo::NetworksInfo.insert(network->ID, network);
        this->close();
        return;
    }
    NetworkInfo *x = NetworkInfo::NetworksInfo[this->networkID];
    x->NetworkName = this->ui->le_Name->text();
    x->Hostname = this->ui->le_HostName->text();
    x->Port = this->ui->le_Port->text().toInt();
    x->SSL = this->ui->checkBox_SSL->isChecked();
    x->PreferredIdentity = this->identityMap[this->ui->cb_IdentityNumber->currentIndex()];

    this->close();
}

void GrumpyIRC::NetworkWin::on_pushExit_clicked()
{
    this->close();
}
