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
#include "scrollbackframe.h"
#include "grumpyconf.h"
#include "../libcore/exception.h"
#include "userframe.h"
#include "userframeitem.h"
#include "scriptwin.h"
#include "ui_userframe.h"
#include <QListWidgetItem>
#include <QMenu>
#include "skin.h"

using namespace GrumpyIRC;

UserFrame::UserFrame(ScrollbackFrame *parent) : QFrame(parent), ui(new Ui::UserFrame)
{
    this->ui->setupUi(this);
    this->parentFrame = parent;
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
    // We have some window selected so let's display a menu
    QPoint globalPos = this->ui->listWidget->viewport()->mapToGlobal(pos);
    QMenu Menu;
    // Items
    QAction *menuQuery = new QAction("PM", &Menu);
    Menu.addAction(menuQuery);
    QAction *menuWhois = new QAction("Whois", &Menu);
    Menu.addAction(menuWhois);

    Menu.addSeparator();

    QMenu *menuModes = new QMenu("Mode", &Menu);

    QAction *modeQ = new QAction("+q", &Menu);
    QAction *modeA = new QAction("+a", &Menu);
    QAction *modeO = new QAction("+o", &Menu);
    QAction *modeH = new QAction("+h", &Menu);
    QAction *modeV = new QAction("+v", &Menu);

    QAction *modeq = new QAction("-q", &Menu);
    QAction *modea = new QAction("-a", &Menu);
    QAction *modeo = new QAction("-o", &Menu);
    QAction *modeh = new QAction("-h", &Menu);
    QAction *modev = new QAction("-v", &Menu);

    menuModes->addAction(modeQ);
    menuModes->addAction(modeA);
    menuModes->addAction(modeO);
    menuModes->addAction(modeH);
    menuModes->addAction(modeV);
    menuModes->addSeparator();
    menuModes->addAction(modeq);
    menuModes->addAction(modea);
    menuModes->addAction(modeo);
    menuModes->addAction(modeh);
    menuModes->addAction(modev);

    Menu.addMenu(menuModes);

    Menu.addSeparator();

    QMenu *menuCT = new QMenu("CTCP", &Menu);
    QAction *ctcpV = new QAction("VERSION", &Menu);
    QAction *ctcpP = new QAction("PING", &Menu);
    //QAction *ctcpT = new QAction
    //
    menuCT->addAction(ctcpV);
    menuCT->addAction(ctcpP);
    Menu.addMenu(menuCT);

    QAction *menuKick = new QAction(QObject::tr("Kick"), &Menu);
    QAction *menuBan = new QAction(QObject::tr("Ban"), &Menu);
    QAction *menuKickBan = new QAction(QObject::tr("Kick and ban"), &Menu);
    QAction *menuIgnore = new QAction(QObject::tr("Ignore"), &Menu);
    Menu.addAction(menuKick);
    Menu.addAction(menuBan);
    Menu.addAction(menuKickBan);
    Menu.addAction(menuIgnore);

    QAction* selectedItem = Menu.exec(globalPos);
    if (!selectedItem)
        return;
    if (selectedItem == modeo)
        this->ChangeMode("-o");
    else if (selectedItem == modeq)
        this->ChangeMode("-q");
    else if (selectedItem == modeh)
        this->ChangeMode("-h");
    else if (selectedItem == modev)
        this->ChangeMode("-v");
    else if (selectedItem == modea)
        this->ChangeMode("-a");
    else if (selectedItem == modeQ)
        this->ChangeMode("+q");
    else if (selectedItem == modeA)
        this->ChangeMode("+a");
    else if (selectedItem == modeO)
        this->ChangeMode("+o");
    else if (selectedItem == modeH)
        this->ChangeMode("+h");
    else if (selectedItem == modeV)
        this->ChangeMode("+v");
    else if (selectedItem == menuWhois)
        this->Whois();
    else if (selectedItem == ctcpP)
        this->ctcp("PING");
    else if (selectedItem == ctcpV)
        this->ctcp("VERSION");
    else if (selectedItem == menuKick)
        this->kick();
    else if (selectedItem == menuBan)
        this->ban();
    else if (selectedItem == menuKickBan)
        this->kb();
}

QString UserFrame::generateKick()
{
    QString results;
    foreach (libircclient::User user, this->SelectedUsers())
    {
        //this->parentFrame->TransferRaw("PRIVMSG " + user.GetNick());
        results += "KICK " + this->parentFrame->GetScrollback()->GetTarget() + " " + user.GetNick() + " :" + CONF->GetDefaultKickReason() + "\n";
    }
    return results;
}

QString UserFrame::generateBan()
{
    QString results;
    if (CONF->Batches())
    {
        QList<libircclient::User> users = this->SelectedUsers();
        while (!users.isEmpty())
        {
            int current_batch_size = 4;
            if (users.count() < current_batch_size)
                current_batch_size = users.count();
            QString mode = "+";
            QString parameters;
            while(current_batch_size-- > 0)
            {
                mode += "b";
                parameters += "*!*@" + (users.at(0).GetHost()) + " ";
                users.removeAt(0);
            }

            results += "MODE " + this->parentFrame->GetScrollback()->GetTarget() + " " + mode + " " + parameters.trimmed() + "\n";
        }
    } else
    {
        foreach (libircclient::User user, this->SelectedUsers())
        {
            //this->parentFrame->TransferRaw("PRIVMSG " + user.GetNick());
            results += "MODE " + this->parentFrame->GetScrollback()->GetTarget() + " +b *!*@" + user.GetHost() + "\n";
        }
    }
    return results;
}

void UserFrame::kick()
{
    QString script = this->generateKick();
    ScriptWin *script_win = new ScriptWin(this->parentFrame);
    script_win->Set(script);
    script_win->show();
}

void UserFrame::changeModes(char prefix, char mode)
{
    QList<libircclient::User> ul = this->SelectedUsers();
    QString results;
    while (!ul.isEmpty())
    {
        int current_batch_size = 4;
        if (ul.count() < current_batch_size)
            current_batch_size = ul.count();
        QString modes = QString(prefix);
        QString parameters;
        while(current_batch_size-- > 0)
        {
            modes += QString(mode);
            parameters += (ul.at(0).GetNick()) + " ";
            ul.removeAt(0);
        }

        results += "MODE " + this->parentFrame->GetScrollback()->GetTarget() + " " + modes + " " + parameters.trimmed() + "\n";
    }
    ScriptWin *script_window = new ScriptWin(this->parentFrame);
    script_window->Set(results);
    script_window->show();
}

void UserFrame::kb()
{
    QString script = this->generateBan() + this->generateKick();
    ScriptWin *script_win = new ScriptWin(this->parentFrame);
    script_win->Set(script);
    script_win->show();
}

void UserFrame::ctcp(QString text)
{
    foreach (libircclient::User user, this->SelectedUsers())
    {
        //this->parentFrame->TransferRaw("PRIVMSG " + user.GetNick());
        this->parentFrame->SendCtcp(user.GetNick(), text, "");
    }
}

void UserFrame::ban()
{
    QString script = this->generateBan();
    ScriptWin *script_win = new ScriptWin(this->parentFrame);
    script_win->Set(script);
    script_win->show();
}

QString UserFrame::GenerateTip(libircclient::User *ux)
{
    QString text = ux->ToString();
    if (ux->GetRealname().length())
        text += "\n" + ux->GetRealname();
    if (ux->IsAway)
        text += "\nIs away now.";

    return text;
}

static QColor getColor(libircclient::User *ux)
{
    if (ux->IsAway)
        return Skin::GetDefault()->UserListAwayColor;

    char cumode = ux->GetHighestCUMode();

    if (cumode == 0 || !Skin::GetDefault()->ModeColors.contains(cumode))
        return Skin::GetDefault()->TextColor;
    
    return Skin::GetDefault()->ModeColors[cumode];
}

void UserFrame::InsertUser(libircclient::User *user, bool bulk)
{
    if (!this->network)
        return;
    if (this->userCounts.contains(user->GetHighestCUMode()))
        this->userCounts[user->GetHighestCUMode()]++;
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
    if (!bulk)
    {
        int index = -1;
        int count = this->ui->listWidget->count();
        while (++index < count)
        {
            if (item->lowerThan(*this->ui->listWidget->item(index)))
                break;
        }
        this->ui->listWidget->insertItem(index, item);
    }
    else
    {
        // will be sorted later
        this->ui->listWidget->addItem(item);
    }

    if (!bulk)
    {
        this->UpdateInfo();
    }
}

void UserFrame::RemoveUser(QString user)
{
    // Lowercase
    user = user.toLower();
    if (!this->users.contains(user))
        return;
    libircclient::User ux = this->users[user];
    if (this->userCounts.contains(ux.GetHighestCUMode()))
        this->userCounts[ux.GetHighestCUMode()]--;
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
    this->InsertUser(user, false);
}

void UserFrame::Sort()
{
    this->ui->listWidget->sortItems();
}

void UserFrame::ChangeNick(QString new_nick, QString old_nick)
{
    old_nick = old_nick.toLower();
    if (!this->users.contains(old_nick))
        return;
    libircclient::User user = this->users[old_nick];
    this->RemoveUser(old_nick);
    user.SetNick(new_nick);
    this->InsertUser(&user, false);
}

void UserFrame::SetNetwork(libircclient::Network *Network)
{
    if (!Network)
        return;
    this->network = Network;
    foreach (char mode, this->network->GetCUModes())
        this->userCounts.insert(mode, 0);
}

QList<libircclient::User> UserFrame::SelectedUsers()
{
    QList<libircclient::User> ul;
    if (!this->network)
        return ul;
    QString channel_name = this->parentFrame->GetScrollback()->GetTarget();
    // Check all users who were selected
    foreach(QListWidgetItem* item, this->ui->listWidget->selectedItems())
    {
        QString nick = item->text().toLower();
        if (nick.isEmpty())
            throw new GrumpyIRC::Exception("Empty nick!?", BOOST_CURRENT_FUNCTION);
        // strip the prefix
        char prefix = nick[0].toLatin1();
        if (this->network->GetChannelUserPrefixes().contains(prefix))
            nick = nick.mid(1);
        if (!this->users.contains(nick))
            throw new GrumpyIRC::Exception("Missing user", BOOST_CURRENT_FUNCTION);

        ul.append(this->users[nick]);
    }
    return ul;
}

QList<QString> UserFrame::GetUsers()
{
    QList<QString> ul;

    foreach (libircclient::User user, this->users.values())
        ul.append(user.GetNick());

    return ul;
}

void UserFrame::ChangeMode(QString mode)
{
    QString channel_name = this->parentFrame->GetScrollback()->GetTarget();
    QList<libircclient::User> users = this->SelectedUsers();
    if (users.size() > 1)
    {
        if (mode.size() < 2)
            throw new Exception("Invalid mode", BOOST_CURRENT_FUNCTION);
        this->changeModes(mode[0].toLatin1(), mode[1].toLatin1());
    } else if (users.size() == 1)
    {
        this->parentFrame->TransferRaw("MODE " + channel_name + " " + mode + " " + users.at(0).GetNick());
    }
}

void UserFrame::Whois()
{
    foreach(libircclient::User u, this->SelectedUsers())
        this->parentFrame->TransferRaw("WHOIS " + u.GetNick());
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
