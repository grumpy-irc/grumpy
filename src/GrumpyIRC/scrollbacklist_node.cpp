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
#include "../libcore/scrollback.h"
#include "scrollbackframe.h"

using namespace GrumpyIRC;

ScrollbackList_Node::ScrollbackList_Node(ScrollbackFrame *sb) : QStandardItem(sb->GetWindowName())
{
    this->scrollback = sb;
    this->UpdateIcon();
}

ScrollbackFrame *ScrollbackList_Node::GetScrollback()
{
    return this->scrollback;
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
                this->setIcon(QIcon(":/icons/img/hash.png"));
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
