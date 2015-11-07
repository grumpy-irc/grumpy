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
    this->IsVisible = false;
    this->NeedsUpdate = false;
    this->ui->label->setPalette(Skin::GetDefault()->Palette());
    this->ui->label->setText("");
    this->ui->listWidget->setPalette(Skin::GetDefault()->Palette());
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

QString UserFrame::GenerateTip(libircclient::User *ux)
{
    QString text = ux->ToString();
    if (ux->GetRealname().length())
        text += "\n" + ux->GetRealname();

    return text;
}

static QColor getColor(libircclient::User *ux)
{
    if (ux->CUMode == 0 || !Skin::GetDefault()->ModeColors.contains(ux->CUMode))
        return Skin::GetDefault()->TextColor;
    
    return Skin::GetDefault()->ModeColors[ux->CUMode];
}

void UserFrame::InsertUser(libircclient::User *user)
{
    if (!this->network)
        return;
    if (this->userCounts.contains(user->CUMode))
        this->userCounts[user->CUMode]++;
    QString name = user->GetNick().toLower();
    if (this->users.contains(name))
        this->RemoveUser(name);
    this->users.insert(name, libircclient::User(user));
    UserFrameItem *item = new UserFrameItem(user->GetPrefixedNick(), this->network);
    item->setToolTip(this->GenerateTip(user));
    item->setTextColor(getColor(user));
    this->userItem.insert(name, item);
    // insert user in a way it's already sorted, so that we don't need to resort the whole
    // user list, which is pretty expensive operation
    int index = -1;
    int count = this->ui->listWidget->count();
    while (++index < count)
    {
        if (item->lowerThan(*this->ui->listWidget->item(index)))
            break;
    }
    this->ui->listWidget->insertItem(index, item);
    /*if (this->IsVisible)
        this->ui->listWidget->sortItems();
    else
        this->NeedsUpdate = true;
    */
    this->UpdateInfo();
}

void UserFrame::RemoveUser(QString user)
{
    // Lowercase
    user = user.toLower();
    if (!this->users.contains(user))
        return;
    libircclient::User ux = this->users[user];
    if (this->userCounts.contains(ux.CUMode))
        this->userCounts[ux.CUMode]--;
    this->users.remove(user);
    this->ui->listWidget->removeItemWidget(this->userItem[user]);
    delete this->userItem[user];
    this->userItem.remove(user);
    this->UpdateInfo();
}

void UserFrame::RefreshUser(libircclient::User *user)
{
    //! \todo Optimize
    this->RemoveUser(user->GetNick());
    this->InsertUser(user);
}

void UserFrame::ChangeNick(QString new_nick, QString old_nick)
{
    old_nick = old_nick.toLower();
    if (!this->users.contains(old_nick))
        return;
    libircclient::User user = this->users[old_nick];
    this->RemoveUser(old_nick);
    user.SetNick(new_nick);
    this->InsertUser(&user);
}

void UserFrame::SetNetwork(libircclient::Network *Network)
{
    if (!Network)
        return;
    this->network = Network;
    foreach (char mode, this->network->GetCUModes())
        this->userCounts.insert(mode, 0);
}

QList<QString> UserFrame::GetUsers()
{
    QList<QString> ul;

    foreach (libircclient::User user, this->users.values())
        ul.append(user.GetNick());

    return ul;

}

void UserFrame::UpdateInfo()
{
    if (!this->IsVisible)
    {
        this->NeedsUpdate = true;
        return;
    }

    if (this->NeedsUpdate)
    {
        //this->ui->listWidget->sortItems();
        this->NeedsUpdate = false;
    }

    // create a overview list
    QString overview = QString::number(this->users.count());
    QString extras = " (";
    bool extras_found = false;
    foreach (char mode, this->userCounts.keys())
    {
        int count = this->userCounts[mode];
        if (!count)
            continue;
        QString info = QChar(mode) + ": ";
        // now count all users with this mode
        info += QString::number(count) + " ";
        extras_found = true;
        extras += info;
    }
    if (extras.endsWith(" "))
        extras = extras.mid(0, extras.size() - 1);
    extras += ")";
    if (extras_found)
        overview += extras;
    this->ui->label->setText(overview);
}
