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
#include "eventhandler.h"
#include "grumpydsession.h"
#include "../libirc/libircclient/network.h"
#include "../libirc/libircclient/user.h"
#include "../libirc/libircclient/channel.h"
#include "ircsession.h"
#include "exception.h"
#include "scrollback.h"
#include <QTcpSocket>
#include <QDataStream>

using namespace GrumpyIRC;

QMutex GrumpydSession::Sessions_Lock;
QList<GrumpydSession*> GrumpydSession::Sessions;

GrumpydSession::GrumpydSession(Scrollback *System, QString Hostname, QString UserName, QString Pass, int Port)
{
    this->systemWindow = System;
    this->hostname = Hostname;
    this->systemWindow->SetSession(this);
    this->port = Port;
    this->username = UserName;
    this->password = Pass;
    this->SSL = false;
    GrumpydSession::Sessions_Lock.lock();
    GrumpydSession::Sessions.append(this);
    GrumpydSession::Sessions_Lock.unlock();
}

GrumpydSession::~GrumpydSession()
{
    delete this->systemWindow;
    GrumpydSession::Sessions_Lock.lock();
    GrumpydSession::Sessions.removeAll(this);
    GrumpydSession::Sessions_Lock.unlock();
}

Scrollback *GrumpydSession::GetSystemWindow()
{
    return this->systemWindow;
}

void GrumpydSession::Open(libirc::ServerAddress server)
{
    QHash<QString, QVariant> parameters;
    parameters.insert("server", QVariant(server.ToHash()));
    this->SendProtocolCommand("SERVER", parameters);
}

bool GrumpydSession::IsConnected() const
{
    return GP::IsConnected();
}

void GrumpydSession::SendMessage(Scrollback *window, QString text)
{

}

libircclient::Network *GrumpydSession::GetNetwork()
{
    return NULL;
}

void GrumpydSession::DelegateCommand(QString command, QString pm, Scrollback *source)
{
    IRCSession *ircs = this->GetSessionFromWindow(source);
    if (!ircs)
        return;

    QHash<QString, QVariant> parameters;
    parameters.insert("network", QVariant(ircs->GetSID()));
    parameters.insert("command", QVariant(command));
    parameters.insert("parameters", QVariant(pm));
    this->SendProtocolCommand("RAW", parameters);
}

SessionType GrumpydSession::GetType()
{
    return SessionType_Grumpyd;
}

Scrollback *GrumpydSession::GetScrollback(unsigned long long original_id)
{
    if (this->scrollbackHash.contains(original_id))
        return this->scrollbackHash[original_id];
    foreach (IRCSession* session, this->sessionList.values())
    {
        Scrollback *sx = session->GetScrollbackByOriginal(original_id);
        if (sx)
        {
            this->scrollbackHash.insert(sx->GetOriginalID(), sx);
            return sx;
        }
    }
    return NULL;
}

IRCSession *GrumpydSession::GetSession(unsigned int nsid)
{
    foreach (IRCSession *session, this->sessionList.values())
    {
        if (session->GetSID() == nsid)
            return session;
    }
    return NULL;
}

IRCSession *GrumpydSession::GetSessionFromWindow(Scrollback *scrollback)
{
    if (this->sessionList.contains(scrollback))
        return this->sessionList[scrollback];
    if (scrollback->GetParentScrollback() && this->sessionList.contains(scrollback->GetParentScrollback()))
        return this->sessionList[scrollback->GetParentScrollback()];

    // There seem to be no irc session associated to this window, it's not our window?
    return NULL;
}

void GrumpydSession::Connect()
{
    if (this->IsConnected())
            return;
    delete this->socket;
    this->systemWindow->InsertText("Connecting to " + this->hostname);
    this->socket = new QTcpSocket();
    this->ResolveSignals();
    connect(this->socket, SIGNAL(connected()), this, SLOT(OnConnected()));
    this->socket->connectToHost(this->hostname, this->port);
}

void GrumpydSession::OnDisconnect()
{

}

void GrumpydSession::OnTimeout()
{

}

void GrumpydSession::OnConnected()
{
    this->systemWindow->InsertText("Connected to remote server, sending HELLO packet");
    QHash<QString, QVariant> parameters;
    parameters.insert("version", QString(GRUMPY_VERSION_STRING));
    this->SendProtocolCommand(GP_CMD_HELLO, parameters);
}

void GrumpydSession::OnIncomingCommand(QString text, QHash<QString, QVariant> parameters)
{
    if (text == GP_CMD_UNKNOWN)
    {
        if (parameters.contains("unrecognized"))
            this->systemWindow->InsertText(QString("Grumpyd didn't recognize this command: ") + parameters["unrecognized"].toString());
    } else if (text == GP_CMD_HELLO)
    {
        if (!parameters.contains("version"))
            return;
        if (!parameters.contains("authentication_required"))
        {
            this->closeError("Remote doesn't support any authentication mechanism");
            return;
        }
        bool authentication_required = parameters["authentication_required"].toBool();
        if (authentication_required && this->username.isEmpty())
        {
            this->closeError("Remote require authentication, but you didn't provide any credentials needed to login");
            return;
        }
        this->systemWindow->InsertText("Received HELLO from remote system, version of server is: " + parameters["version"].toString());
        QHash<QString, QVariant> params;
        params.insert("password", this->password);
        params.insert("username", this->username);
        this->SendProtocolCommand("LOGIN", params);
    } else if (text == GP_CMD_LOGIN_FAIL)
    {
        this->closeError("Invalid username or password provided");
    } else if (text == GP_CMD_CHANNEL_RESYNC)
    {
        this->processChannelResync(parameters);
    } else if (text == GP_CMD_SCROLLBACK_RESYNC)
    {
        this->processSResync(parameters);
    } else if (text == GP_CMD_LOGIN_OK)
    {
        this->systemWindow->InsertText("Synchronizing networks");
        this->SendProtocolCommand(GP_CMD_NETWORK_INFO);
    } else if (text == GP_CMD_SCROLLBACK_LOAD_NEW_ITEM)
    {
        this->processNewScrollbackItem(parameters);
    } else if (text == GP_CMD_NETWORK_INFO)
    {
        this->processNetwork(parameters);
    } else if (text == GP_CMD_PERMDENY)
    {
        QString source = "unknown request";
        if (parameters.contains("source"))
            source = parameters["source"].toString();
        this->systemWindow->InsertText("Permission denied: " + source);
    } else
    {
        QHash<QString, QVariant> params;
        params.insert("source", text);
        this->SendProtocolCommand(GP_CMD_UNKNOWN, params);
        this->systemWindow->InsertText("Unknown command from grumpyd " + text);
    }
}

void GrumpydSession::processNewScrollbackItem(QHash<QString, QVariant> hash)
{
    if (!hash.contains("network_id"))
        return;
    // Fetch the network this item belongs to
    IRCSession *session = NULL;
    unsigned int sid = hash["network_id"].toUInt();
    if (!hash.contains("scrollback"))
    {
        GRUMPY_DEBUG("Missing scrollback id for item", 2);
        return;
    }
    unsigned long long id = hash["scrollback"].toULongLong();
    Scrollback *window = this->GetScrollback(id);
    if (!window)
    {
        this->systemWindow->InsertText("Received scrollback item for scrollback which couldn't be found, name: " + hash["scrollback_name"].toString());
        return;
    }
    window->InsertText(ScrollbackItem(hash["item"].toHash()));
}

void GrumpydSession::processNetwork(QHash<QString, QVariant> hash)
{
    if (!hash.contains("sessions"))
        return;

    // Deserialize all irc sessions
    QList<QVariant> session_list = hash["sessions"].toList();
    foreach (QVariant session_hash, session_list)
    {
        IRCSession *session = new IRCSession(session_hash.toHash(), this->systemWindow);
        session->GetSystemWindow()->SetSession(this);
        this->sessionList.insert(session->GetSystemWindow(), session);
    }
    this->systemWindow->InsertText("Synced networks: " + QString::number(session_list.count()));
}

void GrumpydSession::processChannelResync(QHash<QString, QVariant> hash)
{
    IRCSession *session = this->GetSession(hash["network_id"].toUInt());
    if (!session)
        return;
    libircclient::Channel resynced_channel(hash["channel"].toHash());
    libircclient::Channel *channel = session->GetNetwork()->GetChannel(resynced_channel.GetName());
    // Find a scrollback that is associated to this channel if there is some
    Scrollback *window = session->GetScrollback(resynced_channel.GetName());
    if (!channel)
    {
        // There is no such a channel, so let's insert it to network structure
        channel = session->GetNetwork()->InsertChannel(&resynced_channel);
        // Resync the users with scrollback and quit
        foreach (libircclient::User *user, channel->GetUsers())
        {
            if (window)
                window->UserListChange(user->GetNick(), user, UserListChange_Insert);
        }
        return;
    }

    if (window)
    {
        // Remove all users from the window's internal user list
        foreach (libircclient::User *user, channel->GetUsers())
            window->UserListChange(user->GetNick(), user, UserListChange_Remove);
    } else
    {
        GRUMPY_ERROR("request to resync an existing channel for which there is no window: " + channel->GetName());
    }

    channel->ClearUsers();

    foreach (libircclient::User *user, resynced_channel.GetUsers())
    {
        // we need to insert a copy of user, not just a pointer because the base class will be destroyed on end
        // of this function and so will all its subclasses
        channel->InsertUser(new libircclient::User(user));
        if (window)
            window->UserListChange(user->GetNick(), user, UserListChange_Insert);
    }
}

void GrumpydSession::processSResync(QHash<QString, QVariant> parameters)
{
    // Let's register this scrollback to this session
    Scrollback *root = this->systemWindow;
    unsigned long long parent = this->systemWindow->GetID();
    if (parameters.contains("parent_sid"))
    {
        parent = parameters["parent_sid"].toULongLong();
        root = this->GetScrollback(parent);
        if (!root)
            root = this->systemWindow;
    }
    Scrollback *result = Core::GrumpyCore->NewScrollback(root, parameters["name"].toString(), ScrollbackType_User);
    result->SetSession(this);
    result->LoadHash(parameters["scrollback"].toHash());
    this->scrollbackHash.insert(result->GetOriginalID(), result);
    if (result->GetType() == ScrollbackType_Channel)
    {
        // if this is a channel we also need to fill up some information about the user list
        unsigned int network_id = parameters["network_id"].toUInt();
        // find the network that belongs to this channel
        IRCSession *session = this->GetSession(network_id);
        if (!session)
            return;
        // get a pointer to channel
        result->SetNetwork(session->GetNetwork());
        libircclient::Channel *channel = session->GetNetwork()->GetChannel(result->GetTarget());
        if (!channel)
            return;
        // get a list of users in this channel and fill them up to scrollback info
        foreach (libircclient::User *user, channel->GetUsers())
            result->UserListChange(user->GetNick(), user, UserListChange_Insert);
    }
}

void GrumpydSession::closeError(QString error)
{
    this->Disconnect();
    this->systemWindow->SetDead(true);
    this->systemWindow->InsertText("Connection failure: " + error);
}

