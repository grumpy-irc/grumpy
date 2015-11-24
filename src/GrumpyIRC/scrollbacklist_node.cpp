//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "scrollbacklist_node.h"
#include "skin.h"
#include "../libirc/libircclient/network.h"
#include "../libirc/libircclient/channel.h"
#include "../libcore/scrollback.h"
#include "../libcore/networksession.h"
#include "scrollbackframe.h"

using namespace GrumpyIRC;

ScrollbackList_Node::ScrollbackList_Node(ScrollbackFrame *sb) : QStandardItem(sb->GetWindowName())
{
    this->scrollback = sb;
    this->IsSystem = false;
    this->RebuildCache();
    this->UpdateColor();
    this->UpdateToolTip();
    this->UpdateIcon();
}

ScrollbackFrame *ScrollbackList_Node::GetScrollback()
{
    return this->scrollback;
}

void ScrollbackList_Node::RebuildCache()
{
    this->unreadBrush = this->foreground();
    this->standardBrush = this->foreground();
    this->highlighterBrush = this->foreground();
    this->systemBrush = this->foreground();
    this->unreadBrush.setColor(Skin::GetDefault()->Unread);
    this->standardBrush.setColor(Skin::GetDefault()->TextColor);
    this->highlighterBrush.setColor(Skin::GetDefault()->HighligtedColor);
    this->systemBrush.setColor(Skin::GetDefault()->SystemColor);
}

void ScrollbackList_Node::UpdateIcon()
{
    if (!this->scrollback->IsDeletable)
    {
        this->setIcon(QIcon(":/icons/img/system.png"));
        return;
    }
    if (!this->scrollback->IsDead())
    {
        switch (this->scrollback->GetScrollback()->GetType())
        {
            case ScrollbackType_Channel:
                this->setIcon(QIcon(":/icons/img/icon_hash.png"));
                break;
            case ScrollbackType_System:
                this->setIcon(QIcon(":/icons/img/exclamation mark.png"));
                break;
            case ScrollbackType_User:
                this->setIcon(QIcon(":/icons/img/at.png"));
                break;
        }
    } else
    {
        switch (this->scrollback->GetScrollback()->GetType())
        {
            case ScrollbackType_Channel:
                this->setIcon(QIcon(":/icons/img/hash-s.png"));
                break;
            case ScrollbackType_System:
                this->setIcon(QIcon(":/icons/img/exclamation-mark-s.png"));
                break;
            case ScrollbackType_User:
                this->setIcon(QIcon(":/icons/img/at-s.png"));
                break;
        }
    }
}

void ScrollbackList_Node::UpdateToolTip()
{
    QString tool_tip = "<b>" + this->text() + "</b>";
    if (this->scrollback->IsChannel() && this->scrollback->GetSession())
    {
        libircclient::Channel *channel = this->scrollback->GetSession()->GetChannel(this->scrollback->GetScrollback());
        if (!channel)
            return;
        tool_tip += " Topic: " + channel->GetTopic();
        //tool_tip += "<br>User count: " + channel-
    } else if (this->scrollback->IsNetwork() && this->scrollback->GetSession())
    {
        libircclient::Network *network = this->scrollback->GetSession()->GetNetwork(this->scrollback->GetScrollback());
        if (!network)
            return;
        if (network->IsSSL())
            tool_tip += " Using ssl";
        QString version = network->GetServerVersion();
        if (!version.isEmpty())
        {
            tool_tip += " Version: " + version;
        }
    }
    /*if (this->scrollback->IsDead())
        tool_tip += "<br>Window is dead";*/
    this->setToolTip(tool_tip);
}

void ScrollbackList_Node::UpdateColor()
{
    switch (this->scrollback->GetScrollback()->GetState())
    {
        case ScrollbackState_UnreadMessages:
            this->setForeground(this->unreadBrush);
            break;
        case ScrollbackState_UnreadNotice:
            this->setForeground(this->highlighterBrush);
            break;
        case ScrollbackState_UnreadSystem:
            this->setForeground(this->systemBrush);
            break;
        case ScrollbackState_Normal:
            this->setForeground(this->standardBrush);
            break;
    }
}

bool ScrollbackList_Node::lowerThan(const QStandardItem &other) const
{
    ScrollbackList_Node *node = (ScrollbackList_Node*)&other;
    if (node->IsSystem != this->IsSystem)
        return this->IsSystem;

    return QString::localeAwareCompare(other.text(), this->text()) > 0;
}

bool ScrollbackList_Node::operator<(const QStandardItem &other) const
{
    return this->lowerThan(other);
}
