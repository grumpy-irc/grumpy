//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "scrollbacklist_node.h"
#include "skin.h"
#include <libirc/libircclient/network.h>
#include <libirc/libircclient/channel.h>
#include <libcore/profiler.h>
#include <libcore/exception.h>
#include <libcore/scrollback.h>
#include <libcore/networksession.h>
#include "scrollbackframe.h"

using namespace GrumpyIRC;

QList<ScrollbackList_Node*> ScrollbackList_Node::NodesList;

ScrollbackList_Node::ScrollbackList_Node(ScrollbackFrame *sb) : QStandardItem(sb->GetWindowName()), GrumpyObject("ScrollbackList_Node")
{
    this->scrollback = sb;
    this->IsSystem = false;
    this->RebuildCache();
    this->UpdateColor();
    this->UpdateToolTip();
    this->UpdateIcon();
    NodesList.append(this);
}

ScrollbackList_Node::~ScrollbackList_Node()
{
    NodesList.removeAll(this);
}

ScrollbackFrame *ScrollbackList_Node::GetScrollback()
{
    return this->scrollback;
}

void ScrollbackList_Node::RebuildCache()
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    this->unreadBrush = this->foreground();
    this->standardBrush = this->foreground();
    this->highlighterBrush = this->foreground();
    this->systemBrush = this->foreground();
    this->unreadBrush.setColor(Skin::GetCurrent()->Unread);
    this->standardBrush.setColor(Skin::GetCurrent()->TextColor);
    this->highlighterBrush.setColor(Skin::GetCurrent()->HighligtedColor);
    this->systemBrush.setColor(Skin::GetCurrent()->SystemColor);
}

void ScrollbackList_Node::UpdateIcon()
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
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
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
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
    if (!tool_tip.isEmpty())
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

    bool other_ic = other.text().startsWith('#');
    bool this_ic = this->text().startsWith('#');

    // Channels go first
    if (other_ic && !this_ic)
        return false;
    else if (!other_ic && this_ic)
        return true;

    return QString::localeAwareCompare(other.text(), this->text()) > 0;
}

bool ScrollbackList_Node::operator<(const QStandardItem &other) const
{
    return this->lowerThan(other);
}
