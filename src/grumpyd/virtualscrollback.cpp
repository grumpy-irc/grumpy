//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "../libcore/exception.h"
#include "../libcore/ircsession.h"
#include "../libcore/networksession.h"
#include "../libcore/grumpydsession.h"
#include "virtualscrollback.h"
#include "user.h"
#include "session.h"

using namespace GrumpyIRC;

VirtualScrollback::VirtualScrollback(ScrollbackType Type, Scrollback *parent) : Scrollback(Type, parent)
{
    this->owner = NULL;
}

VirtualScrollback::~VirtualScrollback()
{

}

User *VirtualScrollback::GetOwner() const
{
    return this->owner;
}

void VirtualScrollback::Sync()
{
    if (this->owner == NULL)
        throw new NullPointerException("this->owner", BOOST_CURRENT_FUNCTION);

    // Let's get all sessions who need to be informed about creation of this scrollback
    Session *session = this->owner->GetAnyGPSession();
    if (!session)
        return;
    QHash<QString, QVariant> parameters;
    parameters.insert("name", this->GetTarget());
    if (this->parentSx)
        parameters.insert("parent_sid", QVariant(this->parentSx->GetID()));
    if (this->GetSession() && this->GetSession()->GetType() == SessionType_IRC)
        parameters.insert("network_id", QVariant(((IRCSession*)this->GetSession())->GetSID()));
    parameters.insert("scrollback", QVariant(this->ToHash()));
    session->SendToEverySession(GP_CMD_SCROLLBACK_RESYNC, parameters);
}

void VirtualScrollback::PartialSync()
{
    if (this->owner == NULL)
        throw new NullPointerException("this->owner", BOOST_CURRENT_FUNCTION);

    // Let's get all sessions who need to be informed about resync of this scrollback
    Session *session = this->owner->GetAnyGPSession();
    if (!session)
        return;
    QHash<QString, QVariant> parameters;
    if (this->GetSession() && this->GetSession()->GetType() == SessionType_IRC)
        parameters.insert("network_id", QVariant(((IRCSession*)this->GetSession())->GetSID()));
    parameters.insert("scrollback", QVariant(this->ToPartialHash()));
    session->SendToEverySession(GP_CMD_SCROLLBACK_PARTIAL_RESYNC, parameters);
}

void VirtualScrollback::SetOwner(User *user)
{
    this->owner = user;
}

void VirtualScrollback::InsertText(ScrollbackItem item)
{
    Scrollback::InsertText(item);
    // let's check if this window belongs to a user which has clients
    if (!this->owner)
        throw new Exception("VirtualScrollback NULL owner", BOOST_CURRENT_FUNCTION);
    Session * xx = this->owner->GetAnyGPSession();
    if (!xx)
        return;
    // Deliver information about this message to everyone
    QHash<QString, QVariant> parameters;
    parameters.insert("scrollback", QVariant(this->GetID()));
    parameters.insert("scrollback_name", QVariant(this->GetTarget()));
    if (this->GetSession() && this->GetSession()->GetType() == SessionType_IRC)
        parameters.insert("network_id", QVariant(((IRCSession*)this->GetSession())->GetSID()));
    parameters.insert("item", QVariant(item.ToHash()));
    xx->SendToEverySession(GP_CMD_SCROLLBACK_LOAD_NEW_ITEM, parameters);
}

