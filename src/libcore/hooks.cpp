//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include "core.h"
#include "hooks.h"
#include "scrollback.h"
#include "extension.h"

using namespace GrumpyIRC;

void Hooks::OnScrollback_InsertText(Scrollback *scrollback, ScrollbackItem *item)
{

}

void Hooks::OnScrollback_Destroyed(Scrollback *scrollback)
{
    if (!Core::GrumpyCore)
        return;
    foreach (Extension *e, Core::GrumpyCore->GetExtensions())
    {
        e->Hook_OnScrollbackDestroyed(scrollback);
    }
}

void Hooks::OnNetwork_Disconnect(IRCSession *session)
{
    if (!Core::GrumpyCore)
        return;
    QList<Extension*> ext = Core::GrumpyCore->GetExtensions();
    //  foreach (Extension *e, Core::GrumpyCore->GetExtensions())
    foreach (Extension *e, ext)
    {
        e->Hook_OnNetworkDisconnect(session);
    }
}

void Hooks::OnNetwork_Generic(IRCSession *session, libircclient::Parser *px)
{
    if (!Core::GrumpyCore)
        return;
    foreach (Extension *e, Core::GrumpyCore->GetExtensions())
    {
        e->Hook_OnNetworkGeneric(session, px);
    }
}

void Hooks::OnNetwork_UnknownMessage(IRCSession *session, libircclient::Parser *px)
{
    if (!Core::GrumpyCore)
        return;
    foreach (Extension *e, Core::GrumpyCore->GetExtensions())
    {
        e->Hook_OnNetworkUnknown(session, px);
    }
}

void Hooks::OnNetwork_ChannelJoined(IRCSession *session, const QString& channel_name)
{
    if (!Core::GrumpyCore)
        return;
    foreach (Extension *e, Core::GrumpyCore->GetExtensions())
    {
        e->Hook_OnNetworkChannelJoined(session, channel_name);
    }
}

void Hooks::OnNetwork_ChannelParted(IRCSession *session, libircclient::Parser *px, const QString &channel_name, const QString &reason)
{
    if (!Core::GrumpyCore)
        return;
    foreach (Extension *e, Core::GrumpyCore->GetExtensions())
    {
        e->Hook_OnNetworkChannelParted(session, px, channel_name, reason);
    }
}

void Hooks::OnNetwork_ChannelLeft(IRCSession *session, libircclient::Parser *px, const QString &channel_name, const QString &reason)
{
    if (!Core::GrumpyCore)
        return;
    foreach (Extension *e, Core::GrumpyCore->GetExtensions())
    {
        e->Hook_OnNetworkChannelLeft(session, px, channel_name, reason);
    }
}

void Hooks::OnNetwork_ChannelKicked(IRCSession *session, libircclient::Parser *px, const QString &channel_name, const QString &reason)
{
    if (!Core::GrumpyCore)
        return;
    foreach (Extension *e, Core::GrumpyCore->GetExtensions())
    {
        e->Hook_OnNetworkChannelKicked(session, px, channel_name, reason);
    }
}

bool Hooks::OnNetwork_ChannelTopic(IRCSession *session, Scrollback *scrollback, libircclient::Parser *px, libircclient::Channel *channel, const QString &new_topic, const QString &old_topic)
{
    if (!Core::GrumpyCore)
        return true;
    bool result = true;
    foreach (Extension *e, Core::GrumpyCore->GetExtensions())
    {
        if (!e->Hook_OnNetworkChannelTopic(session, scrollback, px, channel, new_topic, old_topic))
            result = false;
    }
    return result;
}
