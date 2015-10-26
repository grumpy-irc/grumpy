//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "syncableircsession.h"
#include "virtualscrollback.h"
#include "user.h"
#include "session.h"
#include "../libcore/core.h"
#include "../libcore/eventhandler.h"
#include "../libirc/libircclient/parser.h"
#include "../libirc/libirc/serveraddress.h"
#include "../libirc/libircclient/network.h"
#include "../libirc/libircclient/user.h"
#include "../libirc/libircclient/channel.h"
#include "../libcore/exception.h"

using namespace GrumpyIRC;

SyncableIRCSession *SyncableIRCSession::Open(Scrollback *system_window, libirc::ServerAddress &server, User *owner)
{
    if (!server.IsValid())
        throw new GrumpyIRC::Exception("Server object is not valid", BOOST_CURRENT_FUNCTION);
    SyncableIRCSession *sx = new SyncableIRCSession(system_window, owner);
    libircclient::Network *nx = new libircclient::Network(server, server.GetHost());
    sx->Connect(nx);
    return sx;
}

SyncableIRCSession::SyncableIRCSession(QHash<QString, QVariant> sx, User *user, Scrollback *root) : IRCSession(sx, root)
{
    this->owner = user;
    ((VirtualScrollback*)this->systemWindow)->SetOwner(owner);
}

SyncableIRCSession::SyncableIRCSession(Scrollback *system, User *user, Scrollback *root) : IRCSession(system, root)
{
    this->owner = user;
    ((VirtualScrollback*)this->systemWindow)->SetOwner(owner);
}

void SyncableIRCSession::Connect(libircclient::Network *Network)
{
    IRCSession::Connect(Network);
    connect(this->network, SIGNAL(Event_MyInfo(libircclient::Parser*)), this, SLOT(OnInfo(libircclient::Parser*)));
}

void SyncableIRCSession::ResyncChannel(libircclient::Channel *channel)
{
    if (!channel)
        throw new NullPointerException("channel", BOOST_CURRENT_FUNCTION);
    if (!this->owner)
        throw new NullPointerException("this->owner", BOOST_CURRENT_FUNCTION);

    Session *session = this->owner->GetAnyGPSession();
    if (!session)
    {
        // there is nobody we would need to sync this information with
        // so we can safely ignore the request to sync
        // this information is provided through other channels
        return;
    }
    QHash<QString, QVariant> hash;
    hash.insert("network_id", QVariant(this->GetSID()));
    hash.insert("channel_name", QVariant(channel->GetName()));
    hash.insert("channel", QVariant(channel->ToHash()));
    session->SendToEverySession(GP_CMD_CHANNEL_RESYNC, hash);
}

SyncableIRCSession::~SyncableIRCSession()
{

}

void SyncableIRCSession::OnIRCSelfJoin(libircclient::Channel *channel)
{
    IRCSession::OnIRCSelfJoin(channel);
    if (this->channels.contains(channel->GetName().toLower()))
    {
        // Propagate this new window to every connected user
        VirtualScrollback *sx = (VirtualScrollback*)this->channels[channel->GetName().toLower()];
        sx->SetOwner(this->owner);
        sx->Sync();
        // Now sync the channel with all connected users, we need to do this after we sync the window so that client can assign the window pointer to chan
        Session *session = this->owner->GetAnyGPSession();
        if (!session)
            return;
        QHash<QString, QVariant> parameters;
        parameters.insert("scrollback_id", QVariant(sx->GetID()));
        parameters.insert("network_id", QVariant(this->GetSID()));
        parameters.insert("channel", QVariant(channel->ToHash()));
        session->SendToEverySession(GP_CMD_CHANNEL_JOIN, parameters);
    } else
    {
        GRUMPY_ERROR("Sync error, can't resolve scrollback for channel " + channel->GetName() + " system user: " + this->owner->GetName());
    }
}

void SyncableIRCSession::OnIRCJoin(libircclient::Parser *px, libircclient::User *user, libircclient::Channel *channel)
{
    IRCSession::OnIRCJoin(px, user, channel);
    this->ResyncChannel(channel);
}

void SyncableIRCSession::OnNICK(libircclient::Parser *px, QString old_, QString new_)
{
    IRCSession::OnNICK(px, old_, new_);
    Session *session = this->owner->GetAnyGPSession();
    if (!session)
        return;
    QHash<QString, QVariant> parameters;
    parameters.insert("new_nick", QVariant(new_));
    parameters.insert("old_nick", QVariant(old_));
    parameters.insert("network_id", QVariant(this->GetSID()));
    session->SendToEverySession(GP_CMD_NICK, parameters);
}

void SyncableIRCSession::OnIRCSelfNICK(libircclient::Parser *px, QString previous, QString nick)
{
    IRCSession::OnIRCSelfNICK(px, previous, nick);
}

void SyncableIRCSession::OnKICK(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnKICK(px, channel);
    this->ResyncChannel(channel);
}

void SyncableIRCSession::OnPart(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnPart(px, channel);
    this->ResyncChannel(channel);
}

void SyncableIRCSession::OnSelf_KICK(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnSelf_KICK(px, channel);
    this->ResyncChannel(channel);
}

void SyncableIRCSession::OnTOPIC(libircclient::Parser *px, libircclient::Channel *channel, QString previous_one)
{
    IRCSession::OnTOPIC(px, channel, previous_one);
}

void SyncableIRCSession::OnQuit(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnQuit(px, channel);
    this->ResyncChannel(channel);
}

void SyncableIRCSession::OnSelfPart(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnSelfPart(px, channel);
}

void SyncableIRCSession::OnTopicInfo(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnTopicInfo(px, channel);
    //! \todo we need to update the topic on clients as well
}

void SyncableIRCSession::OnInfo(libircclient::Parser *px)
{
    Session *session = this->owner->GetAnyGPSession();
    if (!session)
        return;
    // Resync the whole network
    QHash<QString, QVariant> parameters;
    parameters.insert("network", QVariant(this->GetNetwork()->ToHash()));
    parameters.insert("network_id", QVariant(this->GetSID()));
    session->SendToEverySession(GP_CMD_NETWORK_RESYNC, parameters);
}

void SyncableIRCSession::OnEndOfNames(libircclient::Parser *px)
{
    IRCSession::OnEndOfNames(px);
    libircclient::Channel *channel = this->GetNetwork()->GetChannel(px->GetParameters()[1]);
    if (!channel)
        return;
    this->ResyncChannel(channel);
}
