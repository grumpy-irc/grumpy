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
#include "../libcore/exception.h"
#include "userframeitem.h"

using namespace GrumpyIRC;

UserFrameItem::UserFrameItem(QString text, libircclient::Network *nt) : QListWidgetItem(text)
{
    if (nt == NULL)
        throw new NullPointerException("local libircclient::Network *nt", BOOST_CURRENT_FUNCTION);
    this->network = nt;
}

bool UserFrameItem::operator<(const QListWidgetItem &other) const
{
    return this->lowerThan(other);
}

bool UserFrameItem::lowerThan(const QListWidgetItem &other) const
{
    QString username1 = this->text();
    QString username2 = other.text();
    // in case that one of the users has channel mode, it's higher
    char usermode1 = this->network->StartsWithCUPrefix(username1);
    char usermode2 = this->network->StartsWithCUPrefix(username2);
    if (usermode1 == 0 && usermode2 != 0)
        return false;
    if (usermode1 != 0 && usermode2 == 0)
        return true;
    if (usermode1 != usermode2)
    {
        // They differ by a user mode, so let's sort them by it
        return network->PositionOfUCPrefix(username1[0].toLatin1()) < network->PositionOfUCPrefix(username2[0].toLatin1());
    }
    // They differ by a string name
    return QString::localeAwareCompare(username1, username2) < 0;
}

