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
#include "../libcore/scrollback.h"
#include "scrollbackframe.h"

using namespace GrumpyIRC;

ScrollbackList_Node::ScrollbackList_Node(ScrollbackFrame *sb) : QStandardItem(sb->GetWindowName())
{
    this->scrollback = sb;
    this->RebuildCache();
    this->UpdateColor();
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
