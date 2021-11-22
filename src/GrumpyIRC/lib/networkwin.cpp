//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2021

#include "../../libcore/networkinfo.h"
#include "networkwin.h"
#include "ui_networkwin.h"

using namespace GrumpyIRC;

NetworkWin::NetworkWin(QWidget *parent) : QDialog(parent), ui(new Ui::NetworkWin)
{
    this->ui->setupUi(this);
}

NetworkWin::~NetworkWin()
{
    delete this->ui;
}

void GrumpyIRC::NetworkWin::on_pushSave_clicked()
{
    if (this->networkID < 0)
    {
        NetworkInfo *network = new NetworkInfo(this->ui->le_Name->text(), this->ui->le_HostName->text(), this->ui->le_Port->text().toInt(), -1);
        NetworkInfo::NetworksInfo.insert(network->ID, network);
        this->close();
        return;
    }
    NetworkInfo *x = NetworkInfo::NetworksInfo[this->networkID];
    x->NetworkName = this->ui->le_Name->text();

    this->close();
}

void GrumpyIRC::NetworkWin::on_pushExit_clicked()
{
    this->close();
}
