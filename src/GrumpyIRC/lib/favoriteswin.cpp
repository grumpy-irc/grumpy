//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2021

#include "favoriteswin.h"
#include "grumpyconf.h"
#include "ui_favoriteswin.h"
#include "networkwin.h"
#include "identityeditorwin.h"
#include "mainwindow.h"
#include <libcore/generic.h>
#include <libcore/networkinfo.h>
#include <libcore/identity.h>
#include <QMenu>
#include <algorithm> // Add this include for std::sort

using namespace GrumpyIRC;

void FavoritesWin::Load()
{
    FavoritesWin::loadIdentities();
    FavoritesWin::loadNetworks();
}

void FavoritesWin::Save()
{
    FavoritesWin::saveIdentities();
    FavoritesWin::saveNetworks();
}

FavoritesWin::FavoritesWin(QWidget *parent) :  QDialog(parent), ui(new Ui::FavoritesWin)
{
    this->ui->setupUi(this);

    QStringList heading_1;
    heading_1 << "ID" << "Network name" << "Hostname";
    this->ui->tv_NetworkList->verticalHeader()->setVisible(false);
    this->ui->tv_NetworkList->setColumnCount(heading_1.size());
    this->ui->tv_NetworkList->setShowGrid(false);
    this->ui->tv_NetworkList->setHorizontalHeaderLabels(heading_1);

    this->RefreshNetworks();

    QStringList heading_2;
    heading_2 << "ID" << "Nick" << "Ident" << "Real Name" << "Auto connect";
    this->ui->tv_IdentityList->verticalHeader()->setVisible(false);
    this->ui->tv_IdentityList->setColumnCount(heading_2.size());
    this->ui->tv_IdentityList->setShowGrid(false);
    this->ui->tv_IdentityList->setHorizontalHeaderLabels(heading_2);

    this->RefreshIdentities();
}

FavoritesWin::~FavoritesWin()
{
    delete this->ui;
}

void FavoritesWin::RefreshNetworks()
{
    while (this->ui->tv_NetworkList->rowCount())
        this->ui->tv_NetworkList->removeRow(0);

    int row = 0;
    QList<int> network_list = NetworkInfo::NetworksInfo.keys();
    std::sort(network_list.begin(), network_list.end());

    foreach (int id, network_list)
    {
        NetworkInfo *network = NetworkInfo::NetworksInfo[id];
        this->ui->tv_NetworkList->insertRow(row);
        this->ui->tv_NetworkList->setItem(row, 0, new QTableWidgetItem(QString::number(network->ID)));
        this->ui->tv_NetworkList->setItem(row, 1, new QTableWidgetItem(network->NetworkName));
        this->ui->tv_NetworkList->setItem(row, 2, new QTableWidgetItem(network->Hostname));
        this->ui->tv_NetworkList->setItem(row, 3, new QTableWidgetItem(Generic::Bool2String(network->AutoReconnect)));
        row++;
    }
}

void FavoritesWin::RefreshIdentities()
{
    while(this->ui->tv_IdentityList->rowCount())
        this->ui->tv_IdentityList->removeRow(0);

    int row = 0;
    QList<int> identity_list = Identity::Identities.keys();
    std::sort(identity_list.begin(), identity_list.end());

    foreach (int id, identity_list)
    {
        Identity *identity = Identity::Identities[id];
        this->ui->tv_IdentityList->insertRow(row);
        this->ui->tv_IdentityList->setItem(row, 0, new QTableWidgetItem(QString::number(identity->ID)));
        this->ui->tv_IdentityList->setItem(row, 1, new QTableWidgetItem(identity->Nick));
        this->ui->tv_IdentityList->setItem(row, 2, new QTableWidgetItem(identity->Ident));
        this->ui->tv_IdentityList->setItem(row, 3, new QTableWidgetItem(identity->RealName));
        row++;
    }
}

void GrumpyIRC::FavoritesWin::on_tv_IdentityList_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = this->ui->tv_IdentityList->viewport()->mapToGlobal(pos);
    QMenu Menu;
    // Items
    QAction *menuInsert = new QAction("Insert", &Menu);
    Menu.addAction(menuInsert);
    QAction *menuEdit = new QAction("Edit", &Menu);
    Menu.addAction(menuEdit);
    QAction *menuRemove = new QAction("Remove", &Menu);
    Menu.addAction(menuRemove);

    QList<int> selected = this->selectedIdents();
    if (selected.count() != 1)
    {
        menuEdit->setEnabled(false);
    }


    QAction* selectedItem = Menu.exec(globalPos);
    if (!selectedItem)
        return;
    if (selectedItem == menuInsert)
    {
        IdentityEditorWin iw;
        iw.exec();
        this->RefreshIdentities();
    } else if (selectedItem == menuRemove)
    {
        foreach (int id, selected)
        {
            if (Identity::Identities.contains(id))
            {
                // Remove reference to this identity from all networks
                foreach (int network_id, NetworkInfo::NetworksInfo.keys())
                {
                    if (NetworkInfo::NetworksInfo[network_id]->PreferredIdentity == id)
                        NetworkInfo::NetworksInfo[network_id]->PreferredIdentity = -1;
                }

                delete Identity::Identities[id];
                Identity::Identities.remove(id);
            }
        }
        this->RefreshIdentities();
    } else if (selectedItem == menuEdit)
    {
        IdentityEditorWin iw;
        iw.Load(selected[0]);
        iw.exec();
    }
}

void GrumpyIRC::FavoritesWin::on_tv_NetworkList_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = this->ui->tv_NetworkList->viewport()->mapToGlobal(pos);
    QMenu Menu;
    // Items
    QAction *menuConnect = new QAction("Connect", &Menu);
    Menu.addAction(menuConnect);
    Menu.addSeparator();
    QAction *menuInsert = new QAction("Insert", &Menu);
    Menu.addAction(menuInsert);
    QAction *menuEdit = new QAction("Edit", &Menu);
    Menu.addAction(menuEdit);
    QAction *menuRemove = new QAction("Remove", &Menu);
    Menu.addAction(menuRemove);

    QList<int> selected = this->selectedNetworks();

    if (selected.count() != 1)
    {
        menuConnect->setEnabled(false);
        menuEdit->setEnabled(false);
    }

    QAction* selectedItem = Menu.exec(globalPos);
    if (!selectedItem)
        return;
    if (selectedItem == menuInsert)
    {
        NetworkWin nx;
        nx.exec();
        this->RefreshNetworks();
    } else if (selectedItem == menuRemove)
    {
        foreach (int id, selected)
        {
            if (NetworkInfo::NetworksInfo.contains(id))
            {
                delete NetworkInfo::NetworksInfo[id];
                NetworkInfo::NetworksInfo.remove(id);
            }
        }
        this->RefreshNetworks();
    } else if (selectedItem == menuEdit)
    {
        NetworkWin nx;
        nx.Load(selected[0]);
        nx.exec();
        this->RefreshNetworks();
    } else if (selectedItem == menuConnect)
    {
        NetworkInfo *network = NetworkInfo::NetworksInfo[selected[0]];
        MainWindow::Main->OpenServer(network->ToServerAddress());
        this->close();
    }
}

void FavoritesWin::saveNetworks()
{
    QList<int> networks = NetworkInfo::NetworksInfo.keys();
    QHash<QString, QVariant> list;
    foreach (int id, networks)
    {
        list.insert(QString::number(id), QVariant(NetworkInfo::NetworksInfo[id]->ToHash()));
    }
    CONF->SetNetworks(list);
}

void FavoritesWin::loadNetworks()
{
    QHash<QString, QVariant> nl = CONF->GetNetworks();
    foreach (QString key, nl.keys())
    {
        QHash<QString, QVariant> network_data = nl[key].toHash();
        NetworkInfo *network_info = new NetworkInfo(network_data);
        NetworkInfo::NetworksInfo.insert(network_info->ID, network_info);
    }
}

void FavoritesWin::saveIdentities()
{
    QList<int> identities = Identity::Identities.keys();
    QHash<QString, QVariant> list;
    foreach (int id, identities)
    {
        list.insert(QString::number(id), QVariant(Identity::Identities[id]->ToHash()));
    }
    CONF->SetIdentities(list);
}

void FavoritesWin::loadIdentities()
{
    QHash<QString, QVariant> nl = CONF->GetIdentities();
    foreach (QString key, nl.keys())
    {
        QHash<QString, QVariant> identity_hash = nl[key].toHash();
        Identity *identity = new Identity(identity_hash);
        Identity::Identities.insert(identity->ID, identity);
    }
}

QList<int> FavoritesWin::selectedNetworks()
{
    QList<int> results;
    QList<QTableWidgetItem*> items = this->ui->tv_NetworkList->selectedItems();
    foreach (QTableWidgetItem *xx, items)
    {
        if (xx->column() > 0)
            continue;
        int id = xx->text().toInt();
        if (!results.contains(id))
            results.append(id);
    }

    return results;
}

QList<int> FavoritesWin::selectedIdents()
{
    QList<int> results;
    QList<QTableWidgetItem*> items = this->ui->tv_IdentityList->selectedItems();
    foreach (QTableWidgetItem *xx, items)
    {
        if (xx->column() > 0)
            continue;
        int id = xx->text().toInt();
        if (!results.contains(id))
            results.append(id);
    }

    return results;
}
