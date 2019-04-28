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
#include "userconfiguration.h"
#include "user.h"
#include "grumpyconf.h"
#include "databasebackend.h"
#include "grumpyd.h"
#include "security.h"
#include "session.h"
#include "../libcore/core.h"
#include "../libcore/generic.h"
#include "../libcore/eventhandler.h"
#include "../libcore/exception.h"
#include "../libcore/grumpydsession.h"
#include "../libirc/libircclient/parser.h"
#include "../libirc/libirc/serveraddress.h"
#include "../libirc/libircclient/network.h"
#include "../libirc/libircclient/user.h"
#include "../libirc/libircclient/channel.h"

using namespace GrumpyIRC;

SyncableIRCSession *SyncableIRCSession::Open(Scrollback *system_window, libirc::ServerAddress &server, User *owner)
{
    if (!server.IsValid())
        throw new GrumpyIRC::Exception("Server object is not valid", BOOST_CURRENT_FUNCTION);
    SyncableIRCSession *sx = new SyncableIRCSession(system_window, owner);
    libircclient::Network *nx = new libircclient::Network(server, server.GetHost());
    nx->SetDefaultIdent(owner->GetConfiguration()->GetValueAsString("ident", DEFAULT_IDENT));
    nx->SetDefaultUsername(owner->GetConfiguration()->GetValueAsString("user", "GrumpyChat"));
    sx->Connect(nx);
    Grumpyd::GetBackend()->StoreNetwork(sx);
    return sx;
}

SyncableIRCSession::SyncableIRCSession(QHash<QString, QVariant> sx, User *user, Scrollback *root) : IRCSession(sx, root)
{
    this->owner = user;
    this->snifferEnabled = this->owner->IsAuthorized(PRIVILEGE_USE_SNIFFER);
    this->maxSnifferBufferSize = CONF->GetMaxSnifferSize();
    
    ((VirtualScrollback*)this->systemWindow)->SetOwner(this->owner);
    this->post_init();
}

SyncableIRCSession::SyncableIRCSession(Scrollback *system, User *user, Scrollback *root) : IRCSession(system, root)
{
    this->owner = user;
    this->snifferEnabled = this->owner->IsAuthorized(PRIVILEGE_USE_SNIFFER);
    this->maxSnifferBufferSize = CONF->GetMaxSnifferSize();

    ((VirtualScrollback*)this->systemWindow)->SetOwner(this->owner);
    this->post_init();
}

SyncableIRCSession::SyncableIRCSession(unsigned int id, Scrollback *system, User *user, QList<Scrollback *> sl) : IRCSession(id, system, NULL)
{
    this->owner = user;
    this->snifferEnabled = this->owner->IsAuthorized(PRIVILEGE_USE_SNIFFER);
    this->maxSnifferBufferSize = CONF->GetMaxSnifferSize();

    foreach (Scrollback *sx, sl)
    {
        if (sx->GetType() == ScrollbackType_Channel)
        {
            this->channels.insert(sx->GetTarget().toLower(), sx);
        } else if (sx->GetType() == ScrollbackType_User)
        {
            this->users.insert(sx->GetTarget().toLower(), sx);
        } else
        {
            // System scrollback? Ignoring
        }
    }
    this->post_init();
}

void SyncableIRCSession::Connect(libircclient::Network *Network)
{
    IRCSession::Connect(Network);
}

void SyncableIRCSession::Connect()
{
    this->free();
    delete this->network;
    libirc::ServerAddress server(this->_hostname, this->_ssl, this->_port, this->_nick, this->_password);
    this->network = new libircclient::Network(server, this->_name);
    this->connInternalSocketSignals();
    this->systemWindow->InsertText("Connecting to " + network->GetServerAddress() + ":" + QString::number(server.GetPort()));
    this->systemWindow->SetDead(false);
    foreach (Scrollback *s, this->GetUserScrollbacks())
    {
        s->SetDead(false);

        // Tell clients now
        // let's hope it's not gonna spam hard
        ((VirtualScrollback*)s)->PartialSync();
    }
    QString nick = this->owner->GetConfiguration()->GetValueAsString("nick", "GrumpydUser");
    this->SetNick(nick);
    this->network->SetDefaultNick(nick);
    this->network->SetDefaultIdent(this->owner->GetConfiguration()->GetValueAsString("ident", DEFAULT_IDENT));
    this->network->SetDefaultUsername(this->owner->GetConfiguration()->GetValueAsString("user", "GrumpyChat https://github.com/grumpy-irc/grumpy"));
    this->network->Connect();
    (((VirtualScrollback*)this->systemWindow)->PartialSync());
    this->timerUL.start(this->ulistUpdateTime);
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

User *SyncableIRCSession::GetOwner() const
{
    return this->owner;
}

void SyncableIRCSession::ResyncChannel(libircclient::Channel *channel, QHash<QString, QVariant> cx)
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
    hash.insert("partial", QVariant(true));
    hash.insert("channel_name", QVariant(channel->GetName()));
    hash.insert("channel", QVariant(cx));
    session->SendToEverySession(GP_CMD_CHANNEL_RESYNC, hash);
}

void SyncableIRCSession::Resync(QHash<QString, QVariant> network)
{
    Session *session = this->owner->GetAnyGPSession();
    if (!session)
        return;


    QHash<QString, QVariant> hash;
    hash.insert("network_id", QVariant(this->GetSID()));
    hash.insert("partial", QVariant(true));
    hash.insert("network", QVariant(network));
    session->SendToEverySession(GP_CMD_NETWORK_RESYNC, hash);
}

void SyncableIRCSession::RequestDisconnect(Scrollback *scrollback, QString reason, bool auto_delete)
{
    IRCSession::RequestDisconnect(scrollback, reason, auto_delete);
    // Sync scrollbacks with the clients (at least the dead parameter must be changed)
    foreach (Scrollback *sx, this->users)
        ((VirtualScrollback*)sx)->PartialSync();
    foreach (Scrollback *sx, this->channels)
        ((VirtualScrollback*)sx)->PartialSync();
    ((VirtualScrollback*)this->systemWindow)->PartialSync();
}

Scrollback *SyncableIRCSession::GetScrollbackForUser(QString user)
{
    VirtualScrollback *scrollback = (VirtualScrollback*)IRCSession::GetScrollbackForUser(user);
    if (!scrollback)
        throw new Exception("Underlying session returned NULL scrollback", BOOST_CURRENT_FUNCTION);

    if (scrollback->PropertyBag.contains("initialized"))
        return scrollback;

    // Store in SQL
    Grumpyd::GetBackend()->UpdateNetwork(this);
    scrollback->PropertyBag.insert("initialized", QVariant(true));

    if (!scrollback->GetOwner())
        scrollback->SetOwner(this->owner);

    scrollback->Sync();
    return scrollback;
}

void SyncableIRCSession::SetHostname(const QString& text)
{
    this->_hostname = text;
}

void SyncableIRCSession::SetName(const QString& text)
{
    this->_name = text;
}

void SyncableIRCSession::SetNick(const QString& text)
{
    this->_nick = text;
}

void SyncableIRCSession::SetIdent(const QString& text)
{
    this->_ident = text;
}

void GrumpyIRC::SyncableIRCSession::SetLastNID(unsigned int nid)
{
    IRCSession::lastID = nid + 1;
}

void SyncableIRCSession::SetSSL(bool is_ssl)
{
    this->_ssl = is_ssl;
}

void SyncableIRCSession::SetPort(unsigned int port)
{
    this->_port = port;
}

Configuration *SyncableIRCSession::GetConfiguration()
{
    return this->owner->GetConfiguration();
}

void SyncableIRCSession::SetDisconnected()
{
    IRCSession::SetDisconnected();
    foreach (Scrollback *sx, this->users)
        ((VirtualScrollback*)sx)->PartialSync();
    foreach (Scrollback *sx, this->channels)
        ((VirtualScrollback*)sx)->PartialSync();
    ((VirtualScrollback*)this->systemWindow)->PartialSync();
}

void SyncableIRCSession::connInternalSocketSignals()
{
    IRCSession::connInternalSocketSignals();
    connect(this->network, SIGNAL(Event_MyInfo(libircclient::Parser*)), this, SLOT(OnInfo(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_ISUPPORT(libircclient::Parser*)), this, SLOT(OnServer_ISUPPORT(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_CPMInserted(libircclient::Parser*,libircclient::ChannelPMode,libircclient::Channel*)), this, SLOT(OnPModeInsert(libircclient::Parser*,libircclient::ChannelPMode,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_CPMRemoved(libircclient::Parser*,libircclient::ChannelPMode,libircclient::Channel*)), this, SLOT(OnPModeRemove(libircclient::Parser*,libircclient::ChannelPMode,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_MyInfo(libircclient::Parser*)), this, SLOT(OnInfo(libircclient::Parser*)));
}

void SyncableIRCSession::free()
{
    IRCSession::free();
    this->targetAFKCache.clear();
}

void SyncableIRCSession::OnIRCSelfJoin(libircclient::Channel *channel)
{
    IRCSession::OnIRCSelfJoin(channel);
    if (this->channels.contains(channel->GetName().toLower()))
    {
        // Store in SQL
        Grumpyd::GetBackend()->UpdateNetwork(this);
        // Propagate this new scrollback to every connected user
        VirtualScrollback *sx = (VirtualScrollback*)this->channels[channel->GetName().toLower()];
        if (sx->PropertyBag.contains("initialized"))
        {
            // We joined a channel that already had a scrollback window that we already created in other clients, so let's just resync
            sx->PartialSync();
            return;
        }
        sx->PropertyBag.insert("initialized", QVariant(true));
        if (!sx->GetOwner())
            sx->SetOwner(this->owner);
        sx->Sync();
        // Now sync the channel with all connected users, we need to do this after we sync the scrollback so that client can assign the scrollback pointer to chan
        Session *session = this->owner->GetAnyGPSession();
        if (!session)
            return;
        QHash<QString, QVariant> parameters;
        parameters.insert("scrollback_id", QVariant(sx->GetOriginalID()));
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
    this->resyncUL(channel, GRUMPY_UL_INSERT, user);
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

    QHash<QString, QVariant> hash;
    hash.insert("localUser", QVariant(this->GetNetwork()->GetLocalUserInfo()->ToHash()));
    this->Resync(hash);
}

void SyncableIRCSession::OnKICK(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnKICK(px, channel);
    //this->ResyncChannel(channel);
    if (px->GetParameters().count() < 2)
        return;
    this->resyncULRemove(channel, px->GetParameters()[1]);
}

void SyncableIRCSession::OnPart(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnPart(px, channel);
    //this->ResyncChannel(channel);
    if (!px->GetSourceUserInfo())
        return;
    this->resyncULRemove(channel, px->GetSourceUserInfo()->GetNick());
}

void SyncableIRCSession::OnConnectionFail(QAbstractSocket::SocketError er)
{
    IRCSession::OnConnectionFail(er);
    foreach (Scrollback *sx, this->users)
        ((VirtualScrollback*)sx)->PartialSync();
    foreach (Scrollback *sx, this->channels)
        ((VirtualScrollback*)sx)->PartialSync();
    ((VirtualScrollback*)this->systemWindow)->PartialSync();
}

void SyncableIRCSession::OnSelf_KICK(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnSelf_KICK(px, channel);
    this->ResyncChannel(channel);
}

void SyncableIRCSession::OnTOPIC(libircclient::Parser *px, libircclient::Channel *channel, QString previous_one)
{
    IRCSession::OnTOPIC(px, channel, previous_one);

    if (!channel)
        return;

    QHash<QString, QVariant> cx;
    cx.insert("_topicTime", QVariant(channel->GetTopicTime()));
    cx.insert("_topic", QVariant(channel->GetTopic()));
    this->ResyncChannel(channel, cx);
}

void SyncableIRCSession::OnQuit(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnQuit(px, channel);
    if (!px->GetSourceUserInfo())
        return;
    this->resyncULRemove(channel, px->GetSourceUserInfo()->GetNick());
}

void SyncableIRCSession::OnSelfPart(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnSelfPart(px, channel);
    if (this->channels.contains(channel->GetName().toLower()))
    {
        // Sync the scrollback window for all connected clients
        VirtualScrollback *sx = (VirtualScrollback*)this->channels[channel->GetName().toLower()];
        sx->PartialSync();
    } else
    {
        GRUMPY_ERROR("Sync error, can't resolve scrollback for channel " + channel->GetName() + " system user: " + this->owner->GetName());
    }
}

void SyncableIRCSession::OnTopicInfo(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnTopicInfo(px, channel);

    if (!channel)
        return;

    QHash<QString, QVariant> cx;
    cx.insert("_topicTime", QVariant(channel->GetTopicTime()));
    cx.insert("_topic", QVariant(channel->GetTopic()));
    this->ResyncChannel(channel, cx);
}

void SyncableIRCSession::OnInfo(libircclient::Parser *px)
{
    Q_UNUSED(px);
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

void SyncableIRCSession::OnUMODE(libircclient::Parser *px, libircclient::Channel *channel, libircclient::User *user)
{
    IRCSession::OnUMODE(px, channel, user);
    if (!channel || !user)
        return;

    this->resyncUL(channel, GRUMPY_UL_UPDATE, user);
}

void SyncableIRCSession::OnWHO(libircclient::Parser *px, libircclient::Channel *channel, libircclient::User *user)
{
    IRCSession::OnWHO(px, channel, user);
    if (!channel || !user)
        return;

    this->resyncUL(channel, GRUMPY_UL_UPDATE, user);
}

void SyncableIRCSession::OnPModeInsert(libircclient::Parser *px, libircclient::ChannelPMode mode, libircclient::Channel *channel)
{
    if (!channel)
        return;

    Session *session = this->owner->GetAnyGPSession();
    if (!session)
        return;

    QHash<QString, QVariant> parameters;
    parameters.insert("network_id", QVariant(this->GetSID()));
    parameters.insert("type", QVariant(GP_MODETYPE_PMODE));
    parameters.insert("channel_name", QVariant(channel->GetName()));
    parameters.insert("operation", "insert");
    parameters.insert("mode", QVariant(mode.ToHash()));
    session->SendToEverySession(GP_CMD_RESYNC_MODE, parameters);
}

void SyncableIRCSession::OnPModeRemove(libircclient::Parser *px, libircclient::ChannelPMode mode, libircclient::Channel *channel)
{
    if (!channel)
        return;

    Session *session = this->owner->GetAnyGPSession();
    if (!session)
        return;

    QHash<QString, QVariant> parameters;
    parameters.insert("network_id", QVariant(this->GetSID()));
    parameters.insert("channel_name", QVariant(channel->GetName()));
    parameters.insert("type", QVariant(GP_MODETYPE_PMODE));
    parameters.insert("operation", "remove");
    parameters.insert("mode", QVariant(mode.ToHash()));
    session->SendToEverySession(GP_CMD_RESYNC_MODE, parameters);
}

void SyncableIRCSession::OnMODE(libircclient::Parser *px)
{
    IRCSession::OnMODE(px);

    QHash<QString, QVariant> hash;
    hash.insert("localUserMode", QVariant(this->GetNetwork()->GetLocalUserMode().ToHash()));
    this->Resync(hash);
}

void SyncableIRCSession::OnMODEInfo(libircclient::Parser *px, libircclient::Channel* channel)
{
    IRCSession::OnMODEInfo(px, channel);

    QHash<QString, QVariant> cx;
    cx.insert("localMode", channel->GetMode().ToHash());
    this->ResyncChannel(channel, cx);
}

void SyncableIRCSession::OnChannelMODE(libircclient::Parser *px, libircclient::Channel *channel)
{
    IRCSession::OnChannelMODE(px, channel);

    if (!channel)
        return;

    QHash<QString, QVariant> cx;
    cx.insert("localMode", channel->GetMode().ToHash());
    this->ResyncChannel(channel, cx);
}

void SyncableIRCSession::OnUserAwayStatusChange(libircclient::Parser *px, libircclient::Channel *ch, libircclient::User *ux)
{
    IRCSession::OnUserAwayStatusChange(px, ch, ux);
    this->resyncUL(ch, GRUMPY_UL_UPDATE, ux);
}

static QVariant serializeList(QList<char> data)
{
    QList<QVariant> result;
    foreach (char x, data)
        result.append(QVariant(QChar(x)));
    return QVariant(result);
}

void SyncableIRCSession::OnServer_ISUPPORT(libircclient::Parser *px)
{
    Q_UNUSED(px);
    QHash<QString, QVariant> hash;
    hash.insert("_capabilitiesSubscribed", Generic::QStringListToQVariantList(this->GetNetwork()->GetSubscribedCaps()));
    hash.insert("_capabilitiesSupported", Generic::QStringListToQVariantList(this->GetNetwork()->GetSupportedCaps()));
    hash.insert("CModes", serializeList(this->GetNetwork()->GetCModes()));
    hash.insert("CCModes", serializeList(this->GetNetwork()->GetCCModes()));
    hash.insert("CUModes", serializeList(this->GetNetwork()->GetCUModes()));
    hash.insert("CPModes", serializeList(this->GetNetwork()->GetCPModes()));
    hash.insert("CRModes", serializeList(this->GetNetwork()->GetCRModes()));
    this->Resync(hash);
}

void SyncableIRCSession::OnMessage(libircclient::Parser *px)
{
    IRCSession::OnMessage(px);

    if (this->owner->IsOnline())
        return;

    // In case we have this feature available and current user is offline, we send AFK message
    QString target = px->GetParameters()[0];

    // Check if this isn't a channel message, we don't respond to these
    if (target.toLower() == this->network->GetLocalUserInfo()->GetNick().toLower())
    {
        if (this->owner->GetConfiguration()->GetValueAsBool("offline_ms_bool"))
        {
            // Check if we didn't send this message recently
            if (this->targetAFKCache.contains(target))
            {
                if (this->targetAFKCache[target].secsTo(QDateTime::currentDateTime()) < 120)
                    return;
                this->targetAFKCache[target] = QDateTime::currentDateTime();
            } else
            {
                this->targetAFKCache.insert(target, QDateTime::currentDateTime());
            }
            Scrollback *scrollback = this->GetScrollbackForUser(px->GetSourceUserInfo()->GetNick());
            if (!scrollback)
            {
                GRUMPY_DEBUG("Not sending AFK message to nonexistent scrollback: " + px->GetSourceUserInfo()->GetNick(), 2);
                return;
            }
            // Message
            this->SendMessage(scrollback, this->owner->GetConfiguration()->GetValueAsString("offline_ms_text"));
        }
    }
}

void SyncableIRCSession::post_init()
{
    if (!this->owner)
        throw new NullPointerException("this->owner", BOOST_CURRENT_FUNCTION);
    this->IgnoredNums = this->owner->GetConfiguration()->IgnoredNums();
}

void SyncableIRCSession::rmScrollback(Scrollback *scrollback)
{
    IRCSession::rmScrollback(scrollback);
    // Update the network in database storage
    Grumpyd::GetBackend()->UpdateNetwork(this);
}

void SyncableIRCSession::resyncULRemove(libircclient::Channel *channel, QString user)
{
    Session *session = this->owner->GetAnyGPSession();
    if (!session)
        return;
    // Resync the whole network
    QHash<QString, QVariant> parameters;
    parameters.insert("network_id", QVariant(this->GetSID()));
    parameters.insert("channel_name", QVariant(channel->GetName()));
    parameters.insert("operation", QVariant(GRUMPY_UL_REMOVE));
    parameters.insert("target", QVariant(user));
    session->SendToEverySession(GP_CMD_USERLIST_SYNC, parameters);
}

void SyncableIRCSession::resyncUL(libircclient::Channel *channel, int mode, libircclient::User *user)
{
    Session *session = this->owner->GetAnyGPSession();
    if (!session)
        return;
    // Resync the whole network
    QHash<QString, QVariant> parameters;
    parameters.insert("network_id", QVariant(this->GetSID()));
    parameters.insert("channel_name", QVariant(channel->GetName()));
    parameters.insert("operation", QVariant(mode));
    parameters.insert("user", QVariant(user->ToHash()));
    session->SendToEverySession(GP_CMD_USERLIST_SYNC, parameters);
}
