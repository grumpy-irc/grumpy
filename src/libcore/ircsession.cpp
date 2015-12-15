//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "core.h"
#include "configuration.h"
#include "ircsession.h"
#include "eventhandler.h"
#include "../libirc/libircclient/generic.h"
#include "../libirc/libircclient/parser.h"
#include "../libirc/libirc/serveraddress.h"
#include "../libirc/libircclient/network.h"
#include "../libirc/libircclient/user.h"
#include "../libirc/libircclient/channel.h"
#include "generic.h"
#include "scrollback.h"
#include "exception.h"

using namespace GrumpyIRC;

// ID of network, we start with 2 for debugging purposes as missing id would result in 0 so that we know if it was missing or is proper id
unsigned int IRCSession::lastID = 2;
QMutex IRCSession::Sessions_Lock;
QList<IRCSession*> IRCSession::Sessions;

void IRCSession::Exit(QString message)
{
    foreach (IRCSession *session, Sessions)
    {
        session->RequestDisconnect(NULL, message, true);
    }
}

IRCSession *IRCSession::Open(Scrollback *system_window, libirc::ServerAddress &server, QString network, QString nick, QString ident, QString username)
{
    if (!server.IsValid())
        throw new GrumpyIRC::Exception("Server object is not valid", BOOST_CURRENT_FUNCTION);
    IRCSession *sx = new IRCSession(system_window);
    QString network_name = network;
    if (network.isEmpty())
        network_name = server.GetHost();
    libircclient::Network *nx = new libircclient::Network(server, network_name);
    if (!nick.isEmpty())
        nx->SetDefaultNick(nick);
    if (!ident.isEmpty())
        nx->SetDefaultIdent(ident);
    if (!username.isEmpty())
        nx->SetDefaultUsername(username);
    sx->Connect(nx);
    return sx;
}

IRCSession::IRCSession(QHash<QString, QVariant> sx, Scrollback *root)
{
    this->init(false);
    this->systemWindow = NULL;
    this->Root = root;
    this->LoadHash(sx);
}

IRCSession::IRCSession(Scrollback *system, Scrollback *root)
{
    this->init(false);
    this->Root = root;
    this->systemWindow = system;
    this->systemWindow->SetSession(this);
}

IRCSession::IRCSession(unsigned int id, Scrollback *system, Scrollback *root)
{
    this->init(true);
    this->SID = id;
    this->Root = root;
    this->systemWindow = system;
    this->systemWindow->SetSession(this);
}

IRCSession::~IRCSession()
{
    IRCSession::Sessions_Lock.lock();
    IRCSession::Sessions.removeOne(this);
    IRCSession::Sessions_Lock.unlock();
    qDeleteAll(this->data);
    this->data.clear();
    delete this->network;
}

bool IRCSession::IsAway(Scrollback *scrollback)
{
    Q_UNUSED(scrollback);
    if (!this->network)
        return false;

    return this->network->IsAway();
}

Scrollback *IRCSession::GetSystemWindow()
{
    return this->systemWindow;
}

Scrollback *IRCSession::GetScrollback(QString name)
{
    if (this->systemWindow->GetTarget() == name)
        return this->systemWindow;

    foreach (Scrollback *scrollback, this->users.values())
    {
        if (scrollback->GetTarget() == name)
            return scrollback;
    }

    foreach (Scrollback *scrollback, this->channels.values())
    {
        if (scrollback->GetTarget() == name)
            return scrollback;
    }

    return NULL;
}

Scrollback *IRCSession::GetScrollback(scrollback_id_t sid)
{
    if (this->systemWindow->GetOriginalID() == sid)
        return this->systemWindow;

    foreach (Scrollback *scrollback, this->users.values())
    {
        if (scrollback->GetOriginalID() == sid)
            return scrollback;
    }

    foreach (Scrollback *scrollback, this->channels.values())
    {
        if (scrollback->GetOriginalID() == sid)
            return scrollback;
    }

    return NULL;
}

Scrollback *IRCSession::GetScrollbackByOriginal(scrollback_id_t original_sid)
{
    if (this->systemWindow->GetOriginalID() == original_sid)
        return this->systemWindow;

    foreach (Scrollback *scrollback, this->users.values())
    {
        if (scrollback->GetOriginalID() == original_sid)
            return scrollback;
    }

    foreach (Scrollback *scrollback, this->channels.values())
    {
        if (scrollback->GetOriginalID() == original_sid)
            return scrollback;
    }

    return NULL;
}

libircclient::Network *IRCSession::GetNetwork(Scrollback *window)
{
    Q_UNUSED(window);
    return this->network;
}

QList<Scrollback *> IRCSession::GetScrollbacks()
{
    QList<Scrollback*> sx;
    // Fetch all windows we manage
    sx.append(this->users.values());
    sx.append(this->systemWindow);
    sx.append(this->channels.values());
    return sx;
}

QList<Scrollback *> IRCSession::GetUserScrollbacks()
{
    QList<Scrollback*> sx;

    // Fetch all windows we manage
    sx.append(this->users.values());
    return sx;
}

QList<Scrollback *> IRCSession::GetChannelScrollbacks()
{
    QList<Scrollback*> sx;

    // Fetch all windows we manage
    sx.append(this->channels.values());
    return sx;
}

unsigned int IRCSession::GetSID()
{
    return this->SID;
}

void IRCSession::Connect(libircclient::Network *Network)
{
    this->free();
    if (this->IsConnected())
        throw new Exception("You can't connect to ircsession that is active, disconnect first", BOOST_CURRENT_FUNCTION);

    this->systemWindow->InsertText("Connecting to " + Network->GetServerAddress() + ":" + QString::number(Network->GetPort()));
    this->_hostname = Network->GetServerAddress();
    this->_name = _hostname;
    this->_ssl = Network->IsSSL();
    this->_port = Network->GetPort();
    this->_nick = Network->GetNick();
    delete this->network;
    this->network = Network;
    this->connInternalSocketSignals();
    this->network->Connect();
    this->timerUL.start(this->ulistUpdateTime);
}

void GrumpyIRC::IRCSession::SendMessage(Scrollback * window, QString target, QString text)
{
    if (!this->IsConnected())
    {
        this->GetSystemWindow()->InsertText("Can't send messages to a disconnected network");
        return;
    }
    this->GetNetwork()->SendMessage(text, target);
    // Write the message to active window
    window->InsertText(ScrollbackItem("[ >> " + target + " ]: " + text, ScrollbackItemType_Message, this->GetNetwork()->GetLocalUserInfo(), 0, true));
}

void GrumpyIRC::IRCSession::SendNotice(Scrollback * window, QString target, QString text)
{
    if (!this->IsConnected())
    {
        this->GetSystemWindow()->InsertText("Can't send notices to a disconnected network");
        return;
    }
    this->GetNetwork()->SendNotice(text, target);
    // Write the message to active window
    window->InsertText(ScrollbackItem("[ >> " + target + " ]: " + text, ScrollbackItemType_Notice, this->GetNetwork()->GetLocalUserInfo(), 0, true));
}

bool IRCSession::IsConnected() const
{
    if (this->network && this->network->IsConnected())
        return true;

    return false;
}

void IRCSession::SetNetwork(libircclient::Network *nt)
{
    delete this->network;
    this->network = nt;

    foreach(Scrollback *scrollback, this->GetScrollbacks())
        scrollback->SetNetwork(nt);
}

void IRCSession::SendNotice(Scrollback *window, QString text)
{
    if (!this->IsConnected())
    {
        this->GetSystemWindow()->InsertText("Can't send notices to a disconnected network");
        return;
    }
    if (window->GetTarget().isEmpty())
        throw new GrumpyIRC::Exception("window->GetTarget() contains empty string", BOOST_CURRENT_FUNCTION);
    this->GetNetwork()->SendNotice(text, window->GetTarget());
    // Write the message to active window
    window->InsertText(ScrollbackItem(text, ScrollbackItemType_Notice, this->GetNetwork()->GetLocalUserInfo(), 0, true));
}

Scrollback *IRCSession::GetScrollbackForChannel(QString channel)
{
    channel = channel.toLower();
    if (!this->channels.contains(channel))
        return NULL;

    return this->channels[channel];
}

SessionType IRCSession::GetType()
{
    return SessionType_IRC;
}

QList<QString> IRCSession::GetChannels(Scrollback *window)
{
    Q_UNUSED(window);
    QList<QString> channel_names;
    if (!this->network)
        return channel_names;
    foreach (libircclient::Channel *cx, this->network->GetChannels())
        channel_names.append(cx->GetName());
    return channel_names;
}

void IRCSession::SyncWindows(QHash<QString, QVariant> windows, QHash<QString, Scrollback*> *hash)
{
    // this is most likely a remote IRC session managed by grumpyd because nothing else would
    // deserialize it, but just to be sure we check once more and grab the pointer to grumpyd
    // in case it really is that
    NetworkSession *window_session = this;
    if (this->Root && GrumpyIRC::Generic::IsGrumpy(this->Root))
        window_session = this->Root->GetSession();
    foreach (QVariant xx, windows.values())
    {
        QString name = "unknown_scrollback";
        QHash<QString, QVariant> scrollback_h = xx.toHash();
        if (scrollback_h.contains("_target"))
            name = scrollback_h["_target"].toString();
        Scrollback *scrollback = Core::GrumpyCore->NewScrollback(this->systemWindow, name, ScrollbackType_System);
        scrollback->LoadHash(scrollback_h);
        // Set a pointer to this network so that wrappers that render the user lists can have some direct access to it
        scrollback->SetNetwork(this->GetNetwork());
        if (scrollback->GetType() == ScrollbackType_Channel && !scrollback->IsDead())
        {
            // This is a channel so we need to insert all users to list
            // first get a respective channel from network structure
            libircclient::Channel *channel = this->GetNetwork()->GetChannel(scrollback->GetTarget());
            if (channel)
            {
                foreach (libircclient::User *user, channel->GetUsers())
                    scrollback->UserListChange(user->GetNick(), user, UserListChange_Insert, true);
            }
            scrollback->FinishBulk();
        }
        scrollback->SetSession(window_session);
        hash->insert(name.toLower(), scrollback);
    }
}

bool IRCSession::isRetrievingWhoInfo(QString channel)
{
    return this->retrievingWho.contains(channel.toLower());
}

void IRCSession::init(bool preindexed)
{
    this->_autoReconnect = false;
    this->AutomaticallyRetrieveBanList = true;
    this->_ssl = false;
    this->snifferEnabled = true;
    this->highlightCollector = NULL;
    this->ulistUpdateTime = 20 * 60000;
    connect(&this->timerUL, SIGNAL(timeout()), this, SLOT(OnUpdateUserList()));
    this->maxSnifferBufferSize = 2000;
    this->network = NULL;
    if (!preindexed)
    {
        IRCSession::Sessions_Lock.lock();
        IRCSession::Sessions.append(this);
        this->SID = lastID++;
        IRCSession::Sessions_Lock.unlock();
    }
    this->_port = 0;
}

Scrollback *IRCSession::GetScrollbackForUser(QString user)
{
    QString user_l = user.toLower();
    if (!this->users.contains(user_l))
    {
        Scrollback *sx = Core::GrumpyCore->NewScrollback(this->systemWindow, user, ScrollbackType_User);
        sx->SetSession(this);
        emit this->Event_ScrollbackIsOpen(sx);
        this->users.insert(user_l, sx);
        return sx;
    }

    return this->users[user_l];
}

QHash<QString, QVariant> IRCSession::ToHash(int max_items)
{
    QHash<QString, QVariant> hash;
    SERIALIZE(SID);
    SERIALIZE(_nick);
    SERIALIZE(_name);
    SERIALIZE(_hostname);
    SERIALIZE(_password);
    SERIALIZE(_autoReconnect);
    SERIALIZE(_port);
    SERIALIZE(_ssl);
    if (this->network)
        hash.insert("network", QVariant(this->network->ToHash()));
    QHash<QString, QVariant> channels_hash;
    foreach (QString channel, this->channels.keys())
        channels_hash.insert(channel, QVariant(this->channels[channel]->ToHash(max_items)));
    QHash<QString, QVariant> users_hash;
    foreach (QString user, this->users.keys())
        users_hash.insert(user, QVariant(this->users[user]->ToHash(max_items)));
    hash.insert("channels", QVariant(channels_hash));
    hash.insert("users", QVariant(users_hash));
    hash.insert("systemWindow", QVariant(this->systemWindow->ToHash(max_items)));
    return hash;
}

void IRCSession::LoadHash(QHash<QString, QVariant> hash)
{
    UNSERIALIZE_UINT(SID);
    UNSERIALIZE_BOOL(_autoReconnect);
    UNSERIALIZE_STRING(_nick);
    UNSERIALIZE_BOOL(_ssl);
    UNSERIALIZE_STRING(_name);
    UNSERIALIZE_UINT(_port);
    UNSERIALIZE_STRING(_password);
    UNSERIALIZE_STRING(_hostname);
    if (hash.contains("network"))
        this->network = new libircclient::Network(hash["network"].toHash());

    if (!hash.contains("systemWindow"))
        throw new Exception("Network is missing a root window", BOOST_CURRENT_FUNCTION);

    // we register a new system window and then we load it
    QString name = "Unknown host";
    if (!_name.isEmpty())
        name = _name;
    this->systemWindow = Core::GrumpyCore->NewScrollback(this->Root, name, ScrollbackType_System);
    this->systemWindow->LoadHash(hash["systemWindow"].toHash());

    if (hash.contains("users"))
        this->SyncWindows(hash["users"].toHash(), &this->users);
    if (hash.contains("channels"))
        this->SyncWindows(hash["channels"].toHash(), &this->channels);
}

void IRCSession::SendAction(Scrollback *window, QString text)
{
    if (!this->IsConnected())
    {
        this->GetSystemWindow()->InsertText("Can't send messages to a disconnected network");
        return;
    }
    if (window->GetTarget().isEmpty())
        throw new GrumpyIRC::Exception("window->GetTarget() contains empty string", BOOST_CURRENT_FUNCTION);
    this->GetNetwork()->SendAction(text, window->GetTarget());
    // Write the message to active window
    window->InsertText(ScrollbackItem(text, ScrollbackItemType_Act, this->GetNetwork()->GetLocalUserInfo()));
}

void IRCSession::SendRaw(Scrollback *window, QString raw)
{
    Q_UNUSED(window);
    if (!this->IsConnected())
    {
        this->GetSystemWindow()->InsertText("Can't send raw data to a disconnected network");
        return;
    }
    this->GetNetwork()->TransferRaw(raw);
}

void IRCSession::RequestRemove(Scrollback *window)
{
    if (!window->IsDead() && window->GetType() != ScrollbackType_User)
        return;
    if (window == this->systemWindow)
    {
        // We need to delete everything
        foreach (Scrollback *scrollback, this->users.values())
            this->rmWindow(scrollback);
        foreach (Scrollback *scrollback, this->channels.values())
            this->rmWindow(scrollback);
        this->rmWindow(window);
        return;
    }
    this->rmWindow(window);
}

void IRCSession::RequestReconnect(Scrollback *window)
{
    Q_UNUSED(window);
    if (this->IsConnected())
        return;

    if (!this->network)
        throw new Exception("You can't use this reconnect method for a network that never was connected", BOOST_CURRENT_FUNCTION);

    this->systemWindow->SetDead(false);
    foreach (Scrollback *s, this->GetUserScrollbacks())
        s->SetDead(false);
    this->network->Reconnect();
}

void IRCSession::Query(Scrollback *window, QString target, QString message)
{
    if (!this->IsConnected())
    {
        window->InsertText("You are not connected to any irc server", ScrollbackItemType_SystemError);
        return;
    }
    Scrollback *sx = this->GetScrollbackForUser(target);
    if (sx == NULL)
    {
        this->systemWindow->InsertText("No target for " + target + ": " + message);
        return;
    }
    if (!message.isEmpty())
    {
        sx->InsertText(ScrollbackItem(message, ScrollbackItemType_Message, this->network->GetLocalUserInfo(), 0, true));
        this->network->SendMessage(message, target);
    }
}

libircclient::Channel *IRCSession::GetChannel(Scrollback *window)
{
    if (!this->network)
        return NULL;
    return this->GetNetwork()->GetChannel(window->GetTarget());
}

void IRCSession::RequestDisconnect(Scrollback *window, QString reason, bool auto_delete)
{
    this->_autoReconnect = false;
    Q_UNUSED(window);
    if (!this->IsConnected())
        return;
    // Disconnect the network
    this->SetDead();
    if (this->GetNetwork())
        this->GetNetwork()->Disconnect(reason);
    if (auto_delete)
        delete this;
}

void IRCSession::RequestPart(Scrollback *window)
{
    if (window->GetType() != ScrollbackType_Channel)
        throw new Exception("You can't request part of a window that isn't a channel", BOOST_CURRENT_FUNCTION);
    this->GetNetwork()->RequestPart(window->GetTarget());
}

libircclient::User *IRCSession::GetSelfNetworkID(Scrollback *window)
{
    if (!this->network)
        return NULL;

    // return data from network info
    return this->network->GetLocalUserInfo();
}

void IRCSession::RegisterChannel(libircclient::Channel *channel, Scrollback *window)
{
    this->channels.insert(channel->GetName().toLower(), window);
    if (!this->GetNetwork()->GetChannel(channel->GetName()))
        this->GetNetwork()->_st_InsertChannel(channel);
}

void IRCSession::RetrieveChannelExceptionList(Scrollback *window, QString channel_name)
{
    Q_UNUSED(window);
    if (!this->network)
        return;

    QString ln = channel_name.toLower();
    this->ignoringBans.append(ln);
    this->GetNetwork()->TransferRaw("MODE " + channel_name + " +e", libircclient::Priority_Low);
}

void IRCSession::RetrieveChannelInviteList(Scrollback *window, QString channel_name)
{
    Q_UNUSED(window);
    if (!this->network)
        return;

    QString ln = channel_name.toLower();
    this->ignoringBans.append(ln);
    this->GetNetwork()->TransferRaw("MODE " + channel_name + " +I", libircclient::Priority_Low);
}

void IRCSession::RetrieveChannelBanList(Scrollback *window, QString channel_name)
{
    if (!this->network)
        return;

    QString ln = channel_name.toLower();
    this->ignoringBans.append(ln);
    this->GetNetwork()->TransferRaw("MODE " + channel_name + " +b", libircclient::Priority_Low);
}

QString IRCSession::GetLocalUserModeAsString(Scrollback *window)
{
    Q_UNUSED(window);
    if (!this->network)
        return "";

    // Fetch a current user mode
    return this->GetNetwork()->GetLocalUserMode().ToString();
}

QString IRCSession::GetName() const
{
    return this->_name;
}

QString IRCSession::GetHostname() const
{
    return this->_hostname;
}

QString IRCSession::GetNick() const
{
    return this->_nick;
}

QString IRCSession::GetPassword() const
{
    return this->_password;
}

QString IRCSession::GetIdent() const
{
    return this->_ident;
}

bool IRCSession::UsingSSL() const
{
    return this->_ssl;
}

unsigned int IRCSession::GetPort() const
{
    return this->_port;
}

bool IRCSession::IsAutoreconnect(Scrollback *window)
{
    Q_UNUSED(window);
    return this->_autoReconnect;
}

void IRCSession::SetAutoreconnect(Scrollback *window, bool reconnect)
{
    Q_UNUSED(window);
    this->_autoReconnect = reconnect;
}

void IRCSession::OnOutgoingRawMessage(QByteArray message)
{
    if (!this->snifferEnabled)
        return;
    this->data.append(new NetworkSniffer_Item(message, true));
    while ((unsigned int)this->data.size() > this->maxSnifferBufferSize)
        this->data.removeFirst();
}

void IRCSession::SendMessage(Scrollback *window, QString text)
{
    if (!this->IsConnected())
    {
        this->GetSystemWindow()->InsertText("Can't send messages to a disconnected network");
        return;
    }
    if (window->GetTarget().isEmpty())
        throw new GrumpyIRC::Exception("window->GetTarget() contains empty string", BOOST_CURRENT_FUNCTION);
    this->GetNetwork()->SendMessage(text, window->GetTarget());
    // Write the message to active window
    window->InsertText(ScrollbackItem(text, ScrollbackItemType_Message, this->GetNetwork()->GetLocalUserInfo(), 0, true));
}

void IRCSession::SendCTCP(Scrollback *window, QString target, QString ctcp, QString param)
{
    Q_UNUSED(window);
    if (!this->network)
        return;

    this->network->SendCtcp(ctcp, param, target);
}

void IRCSession::OnIRCSelfJoin(libircclient::Channel *channel)
{
    if (channel->GetName().isEmpty())
        throw new GrumpyIRC::Exception("Invalid channel name", BOOST_CURRENT_FUNCTION);
    QString ln = channel->GetName().toLower();
    // Find a scrollback for this channel somewhere ;)
    Scrollback *window;
    if (this->channels.contains(ln))
    {
        window = this->channels[ln];
        window->SetDead(false);
    }
    else
    {
        window = Core::GrumpyCore->NewScrollback(this->systemWindow, channel->GetName(), ScrollbackType_Channel);
        emit this->Event_ScrollbackIsOpen(window);
        this->channels.insert(ln, window);
        window->SetSession(this);
    }
    // Request some information about users in the channel
    if (!this->retrievingWho.contains(ln))
        this->retrievingWho.append(ln);
    this->GetNetwork()->TransferRaw("MODE " + channel->GetName(), libircclient::Priority_Low);
    if (this->AutomaticallyRetrieveBanList)
        this->RetrieveChannelBanList(NULL, ln);
    this->GetNetwork()->TransferRaw("WHO " + channel->GetName(), libircclient::Priority_Low);
}

void IRCSession::OnIRCSelfNICK(libircclient::Parser *px, QString previous, QString nick)
{
    Q_UNUSED(px);
    this->systemWindow->InsertText("Your nick was changed from " + previous + " to " + nick);
}

void IRCSession::OnKICK(libircclient::Parser *px, libircclient::Channel *channel)
{
    QString l = channel->GetName().toLower();
    if (!this->channels.contains(l))
        return;
    this->channels[l]->UserListChange(px->GetParameters()[1], NULL, UserListChange_Remove);
    this->channels[l]->InsertText(ScrollbackItem("kicked " + px->GetParameters()[1] + " from channel: " + px->GetText(), ScrollbackItemType_Kick, px->GetSourceUserInfo()));
}

void IRCSession::OnCTCP(libircclient::Parser *px, QString ctcp, QString pars)
{
    QString target = px->GetParameters()[0];
    if (ctcp == "ACTION")
    {
        this->processME(px, pars);
        return;
    }
        if (target != this->GetNetwork()->GetNick())
        return;
    if (ctcp == "VERSION")
    {
        if (!px->GetSourceUserInfo())
            return;
        this->GetNetwork()->SendNotice("VERSION Grumpy IRC client " + QString(GRUMPY_VERSION_STRING) + " http://github.com/grumpy-irc", px->GetSourceUserInfo()->GetNick());
    } else if (ctcp == "TIME")
    {
        if (!px->GetSourceUserInfo())
            return;
        this->GetNetwork()->SendNotice("TIME " + QDateTime::currentDateTime().toString(), px->GetSourceUserInfo()->GetNick());
    }
    if (pars.isEmpty())
        this->systemWindow->InsertText("Incoming CTCP from " + px->GetSourceInfo() + ": " + ctcp);
    else
        this->systemWindow->InsertText("Incoming CTCP from " + px->GetSourceInfo() + ": " + ctcp + " " + pars);
}

void IRCSession::OnMOTD(libircclient::Parser *px)
{
    this->systemWindow->InsertText(ScrollbackItem(px->GetText(),ScrollbackItemType_Notice, libircclient::User("MOTD!@")));
}

void IRCSession::OnPart(libircclient::Parser *px, libircclient::Channel *channel)
{
    if (!this->channels.contains(channel->GetName().toLower()))
        return;
    Scrollback *sc = this->channels[channel->GetName().toLower()];
    // Remove the user from scrollback's user list widget
    // we can't pass a pointer to user in a channel object here, because it was already deleted from channel
    sc->UserListChange(px->GetSourceUserInfo()->GetNick(), NULL, UserListChange_Remove);
    sc->InsertText(ScrollbackItem(px->GetText(), ScrollbackItemType_Part, px->GetSourceUserInfo()));
}

void IRCSession::OnSelf_KICK(libircclient::Parser *px, libircclient::Channel *channel)
{
    if (!this->channels.contains(channel->GetName().toLower()))
        return;

    // Write information to channel window
    Scrollback *sc = this->channels[channel->GetName().toLower()];
    sc->InsertText("You were kicked from the channel by " + px->GetSourceUserInfo()->ToString() + ": " + px->GetText());
    // The channel is about to be deleted now, let's do something about it
    sc->SetDead(true);
}

void IRCSession::OnTOPIC(libircclient::Parser *px, libircclient::Channel *channel, QString previous_one)
{
    if (!this->channels.contains(channel->GetName().toLower()))
        return;

    Scrollback *sc = this->channels[channel->GetName().toLower()];
    sc->InsertText(ScrollbackItem(px->GetText(), ScrollbackItemType_Topic, px->GetSourceUserInfo()));
}

void IRCSession::OnTOPICWhoTime(libircclient::Parser *px, libircclient::Channel *channel)
{
    if (channel->GetTopicUser().isEmpty() || !this->channels.contains(channel->GetName().toLower()))
        return;

    Scrollback *sc = this->channels[channel->GetName().toLower()];
    sc->InsertText("Topic set at " + channel->GetTopicTime().toString() + " by " + channel->GetTopicUser());
}

void IRCSession::OnNickConflict(libircclient::Parser *px)
{
    if (px->GetParameters().size() < 2)
        return;
    this->systemWindow->InsertText("Nick already in use: " + px->GetParameters()[1]);
}

void IRCSession::OnQuit(libircclient::Parser *px, libircclient::Channel *channel)
{
    if (!this->channels.contains(channel->GetName().toLower()))
        return;
    Scrollback *sc = this->channels[channel->GetName().toLower()];
    sc->UserListChange(px->GetSourceUserInfo()->GetNick(), NULL, UserListChange_Remove);
    sc->InsertText(ScrollbackItem(px->GetText(), ScrollbackItemType_Quit, px->GetSourceUserInfo()));
}

void IRCSession::OnSelfPart(libircclient::Parser *px, libircclient::Channel *channel)
{
    Q_UNUSED(px);
    if (!this->channels.contains(channel->GetName().toLower()))
        return;

    Scrollback *sc = this->channels[channel->GetName().toLower()];
    sc->InsertText("You left this channel");
    sc->SetDead(true);
}

void IRCSession::OnTopicInfo(libircclient::Parser *px, libircclient::Channel *channel)
{
    if (!this->channels.contains(channel->GetName().toLower()))
        return;

    Scrollback *sc = this->channels[channel->GetName().toLower()];
    sc->InsertText(ScrollbackItem("TOPIC for " + px->GetParameters()[1] + " is: " + px->GetText()));
}

void IRCSession::OnEndOfNames(libircclient::Parser *px)
{
    if (px->GetParameters().count() < 2)
    {
        // Invalid
        return;
    }
    QString channel = px->GetParameters()[1].toLower();
    if (!this->channels.contains(channel))
        return;
    Scrollback *scrollback = this->channels[channel];
    // Get a channel from network
    libircclient::Channel *ch = this->network->GetChannel(channel);
    if (ch == NULL)
        return;
    foreach (libircclient::User *user, ch->GetUsers())
    {
        scrollback->UserListChange(user->GetNick(), user, UserListChange_Insert);
    }
}

void IRCSession::OnWHO(libircclient::Parser *px, libircclient::Channel *channel, libircclient::User *user)
{
    if (!channel || !user)
        return;
    if (this->ignoringWho.contains(channel->GetName().toLower()))
        return;
    if (!this->retrievingWho.contains(channel->GetName().toLower()))
        this->systemWindow->InsertText("WHO: " + px->GetParameterLine() + ": " + px->GetText());
    // The user in respective channel was likely updated by libirc now
    Scrollback *scrollback = this->GetScrollbackForChannel(channel->GetName());
    if (!scrollback)
        return;
    scrollback->UserListChange(user->GetNick(), user, UserListChange_Refresh);
}

void IRCSession::OnNotice(libircclient::Parser *px)
{
    if (px->GetParameters().count() < 1)
    {
        this->systemWindow->InsertText("Ignoring malformed NOTICE: " + px->GetRaw());
        return;
    }
    Scrollback *sx = NULL;
    // Get the target scrollback
    QString target = px->GetParameters()[0];
    // if target is current user we need to find target scrollback based on a user's nick
    if (target.toLower() == this->network->GetLocalUserInfo()->GetNick().toLower())
    {
        if (this->GetConfiguration()->GetValueAsBool("write_notices_to_system", true))
            sx = this->GetSystemWindow();
        else
            sx = this->GetScrollbackForUser(px->GetSourceUserInfo()->GetNick());
    } else
    {
        sx = this->GetScrollbackForChannel(target);
    }
    if (sx == NULL)
    {
        this->systemWindow->InsertText("No target for " + target + ": " + px->GetRaw());
        return;
    }
    sx->InsertText(ScrollbackItem(px->GetText(), ScrollbackItemType_Notice, px->GetSourceUserInfo()));
}

void IRCSession::OnWhoEnd(libircclient::Parser *px)
{
    if (px->GetParameters().size() <= 1)
        return;
    QString name = px->GetParameters()[1].toLower();

    // Remove ignored name
    if (this->ignoringWho.contains(name))
    {
        this->ignoringWho.removeAll(name);
        return;
    }

    if (this->retrievingWho.contains(name))
        this->retrievingWho.removeOne(name);
    else
        this->systemWindow->InsertText("End of WHO for " + name);
}

void IRCSession::OnMODEInfo(libircclient::Parser *px, libircclient::Channel *channel)
{
    QStringList lx = px->GetParameters();
    if (lx.size() < 3)
        return;
    if (!this->channels.contains(lx[1].toLower()))
        return;

    Scrollback *sc = this->channels[lx[1].toLower()];
    sc->InsertText("MODE for " + channel->GetName() + " is: " + lx[2]);
}

void IRCSession::OnMODETIME(libircclient::Parser *px)
{
    QStringList lx = px->GetParameters();
    if (lx.size() < 3)
        return;
    if (!this->channels.contains(lx[1].toLower()))
        return;

    Scrollback *sc = this->channels[lx[1].toLower()];
    sc->InsertText("Channel was created at " + QDateTime::fromTime_t(lx[2].toUInt()).toString());
}

void IRCSession::OnUpdateUserList()
{
    if (!this->network)
        return;
    foreach (libircclient::Channel *channel, this->network->GetChannels())
    {
        this->ignoringWho.append(channel->GetName().toLower());
        this->GetNetwork()->TransferRaw("WHO " + channel->GetName(), libircclient::Priority_Low);
    }
}

void IRCSession::OnMODE(libircclient::Parser *px)
{
    if (!px->GetSourceUserInfo())
        return;
    if (px->GetParameters()[0] == this->GetNetwork()->GetNick())
    {
        QString mode = px->GetText();
        if (mode.isEmpty() && px->GetParameters().count() >= 2)
            mode = px->GetParameters()[1];
        this->systemWindow->InsertText(ScrollbackItem(px->GetSourceInfo() + " set your mode " + mode));
    }
}

void IRCSession::OnUserAwayStatusChange(libircclient::Parser *px, libircclient::Channel *ch, libircclient::User *ux)
{
    Scrollback *scrollback = this->GetScrollbackForChannel(ch->GetName());
    if (!scrollback)
        return;

    scrollback->UserListChange(ux->GetNick(), ux, UserListChange_Refresh);
}

void IRCSession::OnChannelMODE(libircclient::Parser *px, libircclient::Channel *channel)
{
    Scrollback *sx = this->GetScrollback(channel->GetName());
    if (!sx)
        return;

    if (!px->GetSourceUserInfo())
        return;

    // Get the mode string
    QStringList mode = px->GetParameters();
    if (mode.isEmpty())
        return;
    mode.removeFirst();
    QString mode_string;
    foreach (QString mp, mode)
        mode_string += mp + " ";
    mode_string = mode_string.trimmed();

    sx->InsertText(ScrollbackItem(mode_string, ScrollbackItemType_Mode, px->GetSourceUserInfo()));
}

void IRCSession::OnUMODE(libircclient::Parser *px, libircclient::Channel *channel, libircclient::User *user)
{
    Q_UNUSED(px);
    if (!channel || !user)
        return;
    // The user in respective channel was likely updated by libirc now
    Scrollback *scrollback = this->GetScrollbackForChannel(channel->GetName());
    if (!scrollback)
        return;
    scrollback->UserListChange(user->GetNick(), user, UserListChange_Refresh);
}

void IRCSession::OnPMODE(libircclient::Parser *px, char mode)
{
    if (px->GetParameters().size() < 2)
        return;
    QString channel = px->GetParameters()[1].toLower();
    if (this->ignoringBans.contains(channel))
        return;
    this->OnUnknown(px);
}

void IRCSession::OnError(libircclient::Parser *px, QString error)
{
    this->systemWindow->InsertText("Parser error (" + error + "): " + px->GetRaw(), ScrollbackItemType_SystemWarning);
}

void IRCSession::OnEndOfBans(libircclient::Parser *px)
{
    if (px->GetParameters().size() < 2)
        return;
    QString channel = px->GetParameters()[1].toLower();
    if (this->ignoringBans.contains(channel))
        this->ignoringBans.removeOne(channel);
    else
        this->OnUnknown(px);
}

void IRCSession::OnEndOfInvites(libircclient::Parser *px)
{
    if (px->GetParameters().size() < 2)
        return;
    QString channel = px->GetParameters()[1].toLower();
    if (this->ignoringInvites.contains(channel))
        this->ignoringInvites.removeOne(channel);
    else
        this->OnUnknown(px);
}

void IRCSession::OnEndOfExcepts(libircclient::Parser *px)
{
    if (px->GetParameters().size() < 2)
        return;
    QString channel = px->GetParameters()[1].toLower();
    if (this->ignoringExceptions.contains(channel))
        this->ignoringExceptions.removeOne(channel);
    else
        this->OnUnknown(px);
}

void IRCSession::OnGeneric(libircclient::Parser *px)
{
    if (!this->IgnoredNums.contains(px->GetNumeric()))
        this->systemWindow->InsertText(px->GetRaw());
}

void IRCSession::OnServerSideUnknown(libircclient::Parser *px)
{
    this->systemWindow->InsertText(px->GetRaw(), ScrollbackItemType_SystemWarning);
}

void IRCSession::OnCapabilitiesNotSupported()
{
    this->systemWindow->InsertText("This ircd doesn't support IRCv3 protocol, disabling CAP support", ScrollbackItemType_SystemWarning);
}

void IRCSession::processME(libircclient::Parser *px, QString message)
{
    Scrollback *sx = NULL;
    // Get the target scrollback
    QString target = px->GetParameters()[0];
    // if target is current user we need to find target scrollback based on a user's nick
    if (target.toLower() == this->network->GetLocalUserInfo()->GetNick().toLower())
    {
        sx = this->GetScrollbackForUser(px->GetSourceUserInfo()->GetNick());
    } else
    {
        sx = this->GetScrollbackForChannel(target);
    }
    if (sx == NULL)
    {
        this->systemWindow->InsertText("No target for " + target + ": " + px->GetRaw());
        return;
    }
    sx->InsertText(ScrollbackItem(message, ScrollbackItemType_Act, px->GetSourceUserInfo()));
}

void IRCSession::free()
{
    this->ignoringBans.clear();
    this->ignoringExceptions.clear();
    this->ignoringWho.clear();
    this->ignoringInvites.clear();
}

void IRCSession::SetDisconnected()
{
    this->SetDead();
}

void IRCSession::SetDead()
{
    this->timerUL.stop();
    foreach (Scrollback *sx, this->users)
        sx->SetDead(true);
    foreach (Scrollback *sx, this->channels)
        sx->SetDead(true);
    this->systemWindow->SetDead(true);
}

Configuration *IRCSession::GetConfiguration()
{
    return Core::GrumpyCore->GetConfiguration();
}

void IRCSession::connInternalSocketSignals()
{
    connect(this->network, SIGNAL(Event_RawOutgoing(QByteArray)), this, SLOT(OnOutgoingRawMessage(QByteArray)));
    connect(this->network, SIGNAL(Event_ConnectionFailure(QAbstractSocket::SocketError)), this, SLOT(OnConnectionFail(QAbstractSocket::SocketError)));
    connect(this->network, SIGNAL(Event_MOTD(libircclient::Parser*)), this, SLOT(OnMOTD(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_RawIncoming(QByteArray)), this, SLOT(OnIncomingRawMessage(QByteArray)));
    connect(this->network, SIGNAL(Event_Unknown(libircclient::Parser*)), this, SLOT(OnUnknown(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_SelfJoin(libircclient::Channel*)), this, SLOT(OnIRCSelfJoin(libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_Join(libircclient::Parser*, libircclient::User*, libircclient::Channel*)), this, SLOT(OnIRCJoin(libircclient::Parser*, libircclient::User*, libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_Kick(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnKICK(libircclient::Parser*,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_PRIVMSG(libircclient::Parser*)), this, SLOT(OnMessage(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_NICK(libircclient::Parser*,QString,QString)), this, SLOT(OnNICK(libircclient::Parser*,QString,QString)));
    connect(this->network, SIGNAL(Event_PerChannelQuit(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnQuit(libircclient::Parser*,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_Part(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnPart(libircclient::Parser*,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_NOTICE(libircclient::Parser*)), this, SLOT(OnNotice(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_EndOfNames(libircclient::Parser*)), this, SLOT(OnEndOfNames(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_SelfNICK(libircclient::Parser*,QString,QString)), this, SLOT(OnIRCSelfNICK(libircclient::Parser*,QString,QString)));
    connect(this->network, SIGNAL(Event_SelfKick(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnSelf_KICK(libircclient::Parser*,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_SelfPart(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnSelfPart(libircclient::Parser*,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_TOPIC(libircclient::Parser*,libircclient::Channel*,QString)), this, SLOT(OnTOPIC(libircclient::Parser*,libircclient::Channel*,QString)));
    connect(this->network, SIGNAL(Event_TOPICInfo(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnTopicInfo(libircclient::Parser*,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_CTCP(libircclient::Parser*,QString,QString)), this, SLOT(OnCTCP(libircclient::Parser*,QString,QString)));
    connect(this->network, SIGNAL(Event_EndOfWHO(libircclient::Parser*)), this, SLOT(OnWhoEnd(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_WHO(libircclient::Parser*,libircclient::Channel*,libircclient::User*)), this, SLOT(OnWHO(libircclient::Parser*,libircclient::Channel*,libircclient::User*)));
    connect(this->network, SIGNAL(Event_TOPICWhoTime(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnTOPICWhoTime(libircclient::Parser*,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_ModeInfo(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnMODEInfo(libircclient::Parser*,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_CreationTime(libircclient::Parser*)), this, SLOT(OnMODETIME(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_Mode(libircclient::Parser*)), this, SLOT(OnMODE(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_UserAwayStatusChange(libircclient::Parser*,libircclient::Channel*,libircclient::User*)), this, SLOT(OnUserAwayStatusChange(libircclient::Parser*,libircclient::Channel*,libircclient::User*)));
    connect(this->network, SIGNAL(Event_NickCollision(libircclient::Parser*)), this, SLOT(OnNickConflict(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_Welcome(libircclient::Parser*)), this, SLOT(OnUnknown(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_ChannelModeChanged(libircclient::Parser*, libircclient::Channel*)), this, SLOT(OnChannelMODE(libircclient::Parser*,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_ChannelUserModeChanged(libircclient::Parser*, libircclient::Channel*, libircclient::User*)), this, SLOT(OnUMODE(libircclient::Parser*,libircclient::Channel*,libircclient::User*)));
    connect(this->network, SIGNAL(Event_MyInfo(libircclient::Parser*)), this, SLOT(OnUnknown(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_ISUPPORT(libircclient::Parser*)), this, SLOT(OnUnknown(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_EndOfInvites(libircclient::Parser*)), this, SLOT(OnEndOfInvites(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_EndOfBans(libircclient::Parser*)), this, SLOT(OnEndOfBans(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_Broken(libircclient::Parser*,QString)), this, SLOT(OnError(libircclient::Parser*,QString)));
    connect(this->network, SIGNAL(Event_PMode(libircclient::Parser*,char)), this, SLOT(OnPMODE(libircclient::Parser*,char)));
    connect(this->network, SIGNAL(Event_Disconnected()), this, SLOT(OnDisconnect()));
    connect(this->network, SIGNAL(Event_ConnectionError(QString,int)), this, SLOT(OnFailure(QString,int)));
    connect(this->network, SIGNAL(Event_NowAway(libircclient::Parser*)), this, SLOT(OnGeneric(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_UnAway(libircclient::Parser*)), this, SLOT(OnGeneric(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_CAP(libircclient::Parser*)), this, SLOT(OnGeneric(libircclient::Parser*)));
    connect(this->network, SIGNAL(Event_CAP_Timeout()), this, SLOT(OnCapabilitiesNotSupported()));
    connect(this->network, SIGNAL(Event_NUMERIC_UNKNOWN(libircclient::Parser*)), this, SLOT(OnServerSideUnknown(libircclient::Parser*)));
}

void IRCSession::_gs_ResyncNickChange(QString new_, QString old_)
{
    // now check all scrollbacks for this user, we don't need to write any message to scrollback itself, that was already handled
    // by sync subsystem of internal text cache
    // what we need to do is an update of userlist that is associated with the scrollback (in case of GUI irc client)
    // this would be much more easier to implement if we just resynced the whole channel list structure on its every change:
    // one function to rule them all? :) but that would be incredible overhead as we would need to resync on every change
    // while this tiny packet is enough (let's try to be more efficient than others)
    foreach (libircclient::Channel *channel, this->GetNetwork()->GetChannels())
    {
        if (!channel->ContainsUser(old_))
            continue;
        if (!this->channels.contains(channel->GetName().toLower()))
            continue;
        // Get a scrollback window and write a nick change message to it
        libircclient::User *ux = channel->GetUser(old_);
        ux->SetNick(new_);
        Scrollback *scrollback = this->channels[channel->GetName().toLower()];
        scrollback->UserListChange(old_, ux, UserListChange_Alter);
    }
}

void IRCSession::rmWindow(Scrollback *window)
{
    if (window->GetType() != ScrollbackType_User && !window->IsDead())
        throw new Exception("You can't delete window which is alive", BOOST_CURRENT_FUNCTION);
    QString name = window->GetTarget().toLower();
    if (this->channels.contains(name))
        this->channels.remove(name);
    else if (this->users.contains(name))
        this->users.remove(name);
    else if (window == this->systemWindow)
        this->systemWindow = NULL;
    else
        return;
    // We removed the window from all lists
    window->Close();
}

void IRCSession::OnIncomingRawMessage(QByteArray message)
{
    if (!this->snifferEnabled)
        return;
    this->data.append(new NetworkSniffer_Item(message, false));
    while (((unsigned int)this->data.size()) > this->maxSnifferBufferSize)
        this->data.removeFirst();
}

void IRCSession::OnConnectionFail(QAbstractSocket::SocketError er)
{
    this->systemWindow->InsertText("Connection failed: " + libircclient::Generic::ErrorCode2String(er), ScrollbackItemType_SystemError);
    this->SetDead();
}

void IRCSession::OnDisconnect()
{
    this->SetDisconnected();
}

void IRCSession::OnMessage(libircclient::Parser *px)
{
    Scrollback *sx = NULL;
    // Get the target scrollback
    QString target = px->GetParameters()[0];
    // if target is current user we need to find target scrollback based on a user's nick
    if (target.toLower() == this->network->GetLocalUserInfo()->GetNick().toLower())
    {
        sx = this->GetScrollbackForUser(px->GetSourceUserInfo()->GetNick());
    } else
    {
        sx = this->GetScrollbackForChannel(target);
    }
    if (sx == NULL)
    {
        this->systemWindow->InsertText("No target for " + target + ": " + px->GetRaw());
        return;
    }
    sx->InsertText(ScrollbackItem(px->GetText(), ScrollbackItemType_Message, px->GetSourceUserInfo()));
}

void IRCSession::OnFailure(QString reason, int code)
{
    this->systemWindow->InsertText("Connection failed: " + reason + " (ec: " + QString::number(code) + ") ");
}

void IRCSession::OnIRCJoin(libircclient::Parser *px, libircclient::User *user, libircclient::Channel *channel)
{
    if (user == NULL || channel == NULL)
    {
        this->systemWindow->InsertText("Ignoring malformed IRC command (unknown user or channel): " + px->GetRaw());
        return;
    }
    if (!this->channels.contains(channel->GetName().toLower()))
    {
        this->systemWindow->InsertText("Ignoring join to unknown channel: " + channel->GetName());
        GRUMPY_DEBUG(px->GetRaw(), 2);
        return;
    }
    // Write a join message to scrollback for this channel
    Scrollback *scrollback = this->channels[channel->GetName().toLower()];
    scrollback->InsertText(ScrollbackItem(user->ToString(), ScrollbackItemType_Join, user));
    scrollback->UserListChange(user->GetNick(), user, UserListChange_Insert);
}

void IRCSession::OnUnknown(libircclient::Parser *px)
{
    this->systemWindow->InsertText(px->GetRaw());
}

void IRCSession::OnNICK(libircclient::Parser *px, QString old_, QString new_)
{
    // This event is called after the nick is changed in all channels
    foreach (libircclient::Channel *channel, this->GetNetwork()->GetChannels())
    {
        if (!channel->ContainsUser(new_))
            continue;
        if (!this->channels.contains(channel->GetName().toLower()))
            continue;
        // Get a scrollback window and write a nick change message to it
        Scrollback *scrollback = this->channels[channel->GetName().toLower()];
        scrollback->InsertText(ScrollbackItem(new_, ScrollbackItemType_Nick, px->GetSourceUserInfo()));
        scrollback->UserListChange(old_, channel->GetUser(new_), UserListChange_Alter);
    }
}

NetworkSniffer_Item::NetworkSniffer_Item(QByteArray data, bool is_outgoing)
{
    this->_outgoing = is_outgoing;
    this->Text = QString(data);
    this->Time = QDateTime::currentDateTime();
}

QList<GrumpyIRC::NetworkSniffer_Item*> GrumpyIRC::IRCSession::GetSniffer()
{
    return this->data;
}
