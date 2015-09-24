//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "scrollback.h"

using namespace GrumpyIRC;

QList<Scrollback*> Scrollback::ScrollbackList;
QMutex Scrollback::ScrollbackList_Mutex;
unsigned long long Scrollback::lastID = 1;

Scrollback::Scrollback()
{
    this->_maxItems = 800000;
    ScrollbackList_Mutex.lock();
    ScrollbackList.append(this);
    ScrollbackList_Mutex.unlock();
    this->_id = lastID++;
}

Scrollback::~Scrollback()
{
    ScrollbackList_Mutex.lock();
    ScrollbackList.removeOne(this);
    ScrollbackList_Mutex.unlock();
}

unsigned long long Scrollback::GetMaxItemsSize()
{
    return this->_maxItems;
}

unsigned long long Scrollback::GetID()
{
    return this->_id;
}

void Scrollback::SetMaxItemsSize(unsigned long long size)
{
    this->_maxItems = size;
}

void Scrollback::InsertText(ScrollbackItem item)
{
    this->items.append(item);
}

void Scrollback::InsertText(QString text, ScrollbackItemType type)
{
    ScrollbackItem item(text);
    item.SetType(type);
    this->InsertText(item);
}

ScrollbackItem::ScrollbackItem(QString text)
{
    this->_type = ScrollbackItemType_System;
    this->_text = text;
    this->_datetime = QDateTime::currentDateTime();
}

ScrollbackItem::~ScrollbackItem()
{

}

QString ScrollbackItem::GetText() const
{
    return this->_text;
}

ScrollbackItemType ScrollbackItem::GetType() const
{
    return this->_type;
}

QDateTime ScrollbackItem::GetTime() const
{
    return this->_datetime;
}

void ScrollbackItem::SetType(ScrollbackItemType type)
{
    this->_type = type;
}

void ScrollbackItem::SetText(QString text)
{
    this->_text = text;
}
