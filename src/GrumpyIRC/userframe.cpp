//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "../libirc/libircclient/network.h"
#include "userframe.h"
#include "userframeitem.h"
#include "ui_userframe.h"
#include "skin.h"

using namespace GrumpyIRC;

UserFrame::UserFrame(QWidget *parent) : QFrame(parent), ui(new Ui::UserFrame)
{
    this->ui->setupUi(this);
    this->network = NULL;
    this->ui->label->setPalette(Skin::Default->Palette());
    this->ui->label->setText("");
    this->ui->listWidget->setPalette(Skin::Default->Palette());
}

UserFrame::~UserFrame()
{
    delete this->ui;
}

void UserFrame::on_listWidget_doubleClicked(const QModelIndex &index)
{

}

void UserFrame::on_listWidget_clicked(const QModelIndex &index)
{

}

void UserFrame::on_listWidget_customContextMenuRequested(const QPoint &pos)
{

}

static QColor getColor(libircclient::User *ux)
{
    if (ux->CUMode == NULL || !Skin::Default->ModeColors.contains(ux->CUMode))
        return Skin::Default->TextColor;
    
    return Skin::Default->ModeColors[ux->CUMode];
}

void UserFrame::InsertUser(libircclient::User *user)
{
    QString name = user->GetNick().toLower();
    if (this->users.contains(name))
        this->RemoveUser(name);
    this->users.insert(name, libircclient::User(user));
    QListWidgetItem *item = new UserFrameItem(user->GetPrefixedNick(), this->network);
    item->setTextColor(getColor(user));
    this->userItem.insert(name, item);
    this->ui->listWidget->addItem(item);
    this->ui->listWidget->sortItems();
    this->UpdateInfo();
}

void UserFrame::RemoveUser(QString user)
{
    // Lowercase
    user = user.toLower();
    if (!this->users.contains(user))
        return;
    this->users.remove(user);
    this->ui->listWidget->removeItemWidget(this->userItem[user]);
    delete this->userItem[user];
    this->userItem.remove(user);
    this->UpdateInfo();
}

void UserFrame::ChangeNick(QString new_nick, QString old_nick)
{
    old_nick = old_nick.toLower();
    if (!this->users.contains(old_nick))
        return;
    libircclient::User user = this->users[old_nick];
    this->users.remove(old_nick);
    user.SetNick(new_nick);
    QString ln = new_nick.toLower();
    this->users.insert(ln, user);
    QListWidgetItem *item = this->userItem[old_nick];
    this->userItem.insert(ln, item);
    this->userItem.remove(old_nick);
    item->setText(new_nick);
}

void UserFrame::SetNetwork(libircclient::Network *Network)
{
    this->network = Network;
}

void UserFrame::UpdateInfo()
{
    // create a overview list
    QString overview = QString::number(this->users.count());
    QString extras = " (";
    bool extras_found = false;
    foreach (char mode, this->network->GetCUModes())
    {
        int count = 0;
        QString info = QChar(mode) + ": ";
        // now count all users with this mode
        foreach (libircclient::User user, this->users.values())
        {
            if (user.CUMode == mode)
                count++;
        }
        info += QString::number(count) + " ";
        if (count)
        {
            extras_found = true;
            extras += info;
        }
    }
    if (extras.endsWith(" "))
        extras = extras.mid(0, extras.size() - 1);
    extras += ")";
    if (extras_found)
        overview += extras;
    this->ui->label->setText(overview);
}
