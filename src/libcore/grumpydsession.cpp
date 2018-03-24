    //This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "../libirc/libircclient/network.h"
#include "../libirc/libircclient/user.h"
#include "../libirc/libircclient/channel.h"
#include "core.h"
#include "configuration.h"
#include "eventhandler.h"
#include "grumpydsession.h"
#include "ircsession.h"
#include "exception.h"
#include "generic.h"
#include "scrollback.h"
#include <QTcpSocket>
#include <QDataStream>

using namespace GrumpyIRC;

QMutex GrumpydSession::Sessions_Lock;
QList<GrumpydSession*> GrumpydSession::Sessions;

GrumpydSession::GrumpydSession(Scrollback *System, QString Hostname, QString UserName, QString Pass, int Port, bool ssl)
{
    this->gp = NULL;
    this->systemWindow = System;
    this->AutoReconnect = true;
    this->hostname = Hostname;
    this->systemWindow->SetSession(this);
    this->systemWindow->SetHidable(false);
    this->port = Port;
    this->syncInit = QDateTime::currentDateTime();
    this->username = UserName;
    this->syncing = false;
    this->password = Pass;
    this->SSL = ssl;
    this->lastUserListUpdate = QDateTime::currentDateTime();
    GrumpydSession::Sessions_Lock.lock();
    GrumpydSession::Sessions.append(this);
    GrumpydSession::Sessions_Lock.unlock();
    Core::GrumpyCore->GetCurrentEventHandler()->OnGrumpydCtorCall(this);
}

GrumpydSession::~GrumpydSession()
{
    Core::GrumpyCore->GetCurrentEventHandler()->OnGrumpydDtorCall(this);
    this->systemWindow->Close();
    GrumpydSession::Sessions_Lock.lock();
    GrumpydSession::Sessions.removeAll(this);
    GrumpydSession::Sessions_Lock.unlock();
    delete this->gp;
}

Scrollback *GrumpydSession::GetSystemWindow()
{
    return this->systemWindow;
}

void GrumpydSession::Open(libirc::ServerAddress server)
{
    QHash<QString, QVariant> parameters;
    parameters.insert("server", QVariant(server.ToHash()));
    this->gp->SendProtocolCommand(GP_CMD_SERVER, parameters);
}

bool GrumpydSession::IsConnected() const
{
    if (!this->gp)
        return false;

    // Figure out from underlying gp info
    return this->gp->IsConnected();
}

void GrumpydSession::SendMessage(Scrollback *window, QString text)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return;
    QHash<QString, QVariant> parameters;
    parameters.insert("type", QVariant(GP_MESSAGETYPE_NORMAL));
    parameters.insert("network_id", QVariant(ircs->GetSID()));
    parameters.insert("scrollback_id", QVariant(window->GetOriginalID()));
    parameters.insert("text", QVariant(text));
    this->gp->SendProtocolCommand(GP_CMD_MESSAGE, parameters);
}

libircclient::Network *GrumpydSession::GetNetwork(Scrollback *window)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return NULL;

    return ircs->GetNetwork();
}

void GrumpydSession::SendRaw(Scrollback *window, QString raw, libircclient::Priority pr)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
    {
        GRUMPY_ERROR("Unknown command");
        return;
    }

    QHash<QString, QVariant> parameters;
    parameters.insert("network_id", QVariant(ircs->GetSID()));
    parameters.insert("priority", QVariant(static_cast<int>(pr)));
    parameters.insert("command", QVariant(raw));
    this->gp->SendProtocolCommand(GP_CMD_RAW, parameters);
}

SessionType GrumpydSession::GetType()
{
    return SessionType_Grumpyd;
}

bool GrumpyIRC::GrumpydSession::IsAway(Scrollback *scrollback)
{
    if (!scrollback)
        return false;
    IRCSession *ircs = this->GetSessionFromWindow(scrollback);
    if (!ircs)
        return false;
    return ircs->IsAway();
}

void GrumpydSession::SendAction(Scrollback *window, QString text)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return;
    QHash<QString, QVariant> parameters;
    parameters.insert("type", QVariant(GP_MESSAGETYPE_ACTION));
    parameters.insert("network_id", QVariant(ircs->GetSID()));
    parameters.insert("scrollback_id", QVariant(window->GetOriginalID()));
    parameters.insert("text", QVariant(text));
    this->gp->SendProtocolCommand(GP_CMD_MESSAGE, parameters);
}

void GrumpydSession::RequestRemove(Scrollback *window)
{
    if (!window->IsDead() && window->GetType() != ScrollbackType_User)
        return;

    if (window == this->systemWindow)
    {
        foreach(IRCSession *nw, this->sessionList.values())
        {
            nw->RequestRemove(nw->GetSystemWindow());
            delete nw;
        }
        this->sessionList.clear();
        this->scrollbackHash.clear();
        delete this->gp;
        this->gp = NULL;
        return;
    }
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return;
    // This is fairly complex, we can't remove window here and keep it in grumpyd, so we need to remotely request its removal
    QHash<QString, QVariant> parameters;
    parameters.insert("scrollback_id", QVariant(window->GetOriginalID()));
    parameters.insert("network_id", ircs->GetSID());
    this->SendProtocolCommand(GP_CMD_REMOVE, parameters);
}

QList<QString> GrumpydSession::GetChannels(Scrollback *window)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return QList<QString>();

    return ircs->GetChannels(window);
}

void GrumpydSession::RequestDisconnect(Scrollback *window, QString reason, bool auto_delete)
{
    if (!this->IsConnected())
        return;
    if (window == this->systemWindow)
    {
        this->AutoReconnect = false;
        // User wants to disconnect whole grumpyd session
        this->kill();
        this->gp->Disconnect();
        if (auto_delete)
            delete this;
    } else
    {
        IRCSession *ircs = this->GetSessionFromWindow(window);
        if (!ircs)
            return;
        QHash<QString, QVariant> parameters;
        parameters.insert("network_id", QVariant(ircs->GetSID()));
        parameters.insert("reason", QVariant(reason));
        this->gp->SendProtocolCommand(GP_CMD_IRC_QUIT, parameters);
    }
}

void GrumpyIRC::GrumpydSession::ResyncPB(Scrollback *window)
{
    QHash<QString, QVariant> parameters;
    parameters.insert("scrollback_id", window->GetOriginalID());
    parameters.insert("property_bag", window->PropertyBag);
    this->gp->SendProtocolCommand(GP_CMD_RESYNC_SCROLLBACK_PB, parameters);
}

void GrumpyIRC::GrumpydSession::ResyncSingleItemPB(Scrollback * window, QString name)
{
    QHash<QString, QVariant> single_item_pb;

    if (!window->PropertyBag.contains(name))
        return;

    single_item_pb.insert(name, window->PropertyBag[name]);

    QHash<QString, QVariant> parameters;
    parameters.insert("scrollback_id", window->GetOriginalID());
    parameters.insert("property_bag", single_item_pb);
    this->gp->SendProtocolCommand(GP_CMD_RESYNC_SCROLLBACK_PB, parameters);
}

void GrumpydSession::RequestPart(Scrollback *window)
{
    if (window->GetType() != ScrollbackType_Channel)
        throw new Exception("You can't request part of a window that isn't a channel", BOOST_CURRENT_FUNCTION);

    // Let's just send a part command, no point in having an extra protocol command for this
    this->SendRaw(window, "PART " + window->GetTarget());
}

void GrumpydSession::Query(Scrollback *window, QString target, QString message)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return;
    QHash<QString, QVariant> parameters;
    parameters.insert("network_id", QVariant(ircs->GetSID()));
    parameters.insert("scrollback_id", QVariant(window->GetOriginalID()));
    if (!message.isEmpty())
        parameters.insert("message", message);
    parameters.insert("target", target);
    this->gp->SendProtocolCommand(GP_CMD_QUERY, parameters);
}

libircclient::Channel *GrumpydSession::GetChannel(Scrollback *window)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return NULL;

    return ircs->GetChannel(window);
}

Scrollback *GrumpydSession::GetScrollback(scrollback_id_t original_id)
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

void GrumpydSession::SendNotice(Scrollback *window, QString text)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return;
    QHash<QString, QVariant> parameters;
    parameters.insert("type", QVariant(GP_MESSAGETYPE_NOTICE));
    parameters.insert("network_id", QVariant(ircs->GetSID()));
    parameters.insert("scrollback_id", QVariant(window->GetOriginalID()));
    parameters.insert("text", QVariant(text));
    this->gp->SendProtocolCommand(GP_CMD_MESSAGE, parameters);
}

void GrumpyIRC::GrumpydSession::SendMessage(Scrollback * window, QString target, QString message)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return;
    QHash<QString, QVariant> parameters;
    parameters.insert("type", QVariant(GP_MESSAGETYPE_NORMAL));
    parameters.insert("target", target);
    parameters.insert("network_id", QVariant(ircs->GetSID()));
    parameters.insert("scrollback_id", QVariant(window->GetOriginalID()));
    parameters.insert("text", QVariant(message));
    this->gp->SendProtocolCommand(GP_CMD_MESSAGE, parameters);
}

void GrumpydSession::SendCTCP(Scrollback *window, QString target, QString ctcp, QString param)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return;
    QHash<QString, QVariant> parameters;
    parameters.insert("type", QVariant(GP_MESSAGETYPE_ISCTCP));
    parameters.insert("network_id", QVariant(ircs->GetSID()));
    parameters.insert("scrollback_id", QVariant(window->GetOriginalID()));
    parameters.insert("name", QVariant(ctcp));
    parameters.insert("target", QVariant(target));
    parameters.insert("text", QVariant(param));
    this->gp->SendProtocolCommand(GP_CMD_MESSAGE, parameters);
}

void GrumpyIRC::GrumpydSession::SendNotice(Scrollback * window, QString target, QString message)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return;
    QHash<QString, QVariant> parameters;
    parameters.insert("type", QVariant(GP_MESSAGETYPE_NOTICE));
    parameters.insert("network_id", QVariant(ircs->GetSID()));
    parameters.insert("scrollback_id", QVariant(window->GetOriginalID()));
    parameters.insert("target", target);
    parameters.insert("text", QVariant(message));
    this->gp->SendProtocolCommand(GP_CMD_MESSAGE, parameters);
}

void GrumpydSession::SendProtocolCommand(unsigned int command)
{
    this->SendProtocolCommand(command, QHash<QString, QVariant>());
}

void GrumpydSession::SendProtocolCommand(unsigned int command, QHash<QString, QVariant> parameters)
{
    if (!this->gp)
    {
        throw new NullPointerException("gp", BOOST_CURRENT_FUNCTION);
    }

    this->gp->SendProtocolCommand(command, parameters);
}

void GrumpydSession::RequestBL(Scrollback *window, scrollback_id_t from, unsigned int size)
{
    QHash<QString, QVariant> parameters;
    parameters.insert("from", QVariant(from));
    parameters.insert("scrollback_id", QVariant(window->GetOriginalID()));
    parameters.insert("request_size", QVariant(size));
    this->gp->SendProtocolCommand(GP_CMD_REQUEST_ITEMS, parameters);
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

QString GrumpydSession::GetLocalUserModeAsString(Scrollback *window)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return "";

    return ircs->GetLocalUserModeAsString(window);
}

void GrumpydSession::RetrieveChannelBanList(Scrollback *window, QString channel_name)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return;

    QHash<QString, QVariant> info;
    info.insert("network_id", QVariant(ircs->GetSID()));
    info.insert("channel_name", QVariant(channel_name));
    info.insert("type", "+b");
    this->SendProtocolCommand(GP_CMD_REQUEST_INFO, info);
}

void GrumpydSession::RequestReconnect(Scrollback *window)
{
    if (window == this->systemWindow)
    {
        // Reconnect grumpyd
        foreach(IRCSession *nw, this->sessionList.values())
            nw->RequestRemove(nw->GetSystemWindow());
        this->sessionList.clear();
        this->scrollbackHash.clear();
        delete this->gp;
        this->gp = NULL;
        this->Connect();
        return;
    }

    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return;
    QHash<QString, QVariant> hash;
    hash.insert("network_id", QVariant(ircs->GetSID()));
    this->SendProtocolCommand(GP_CMD_RECONNECT, hash);
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
    this->connectedOn = QDateTime::currentDateTime();
    this->gp = new libgp::GP();
    connect(this->gp, SIGNAL(Event_Connected()), this, SLOT(OnConnected()));
    connect(this->gp, SIGNAL(Event_Disconnected()), this, SLOT(OnDisconnect()));
    connect(this->gp, SIGNAL(Event_IncomingCommand(gp_command_t, QHash<QString, QVariant>)), this, SLOT(OnIncomingCommand(gp_command_t, QHash<QString, QVariant>)));
    connect(this->gp, SIGNAL(Event_ConnectionFailed(QString,int)), this, SLOT(OnError(QString,int)));
    connect(this->gp, SIGNAL(Event_SslHandshakeFailure(QList<QSslError>, bool*)), this, SLOT(OnSslHandshakeFailure(QList<QSslError>, bool*)));
    this->gp->SetCompression(6);
    this->systemWindow->SetDead(false);
    this->systemWindow->InsertText("Connecting to " + this->hostname + " using port " + QString::number(this->port) + " SSL: " + Generic::Bool2String(this->SSL));
    // Connect grumpy
    this->gp->Connect(this->hostname, this->port, this->SSL);
}

bool GrumpydSession::IsAutoreconnect(Scrollback *window)
{
    if (window == this->systemWindow)
        return this->AutoReconnect;

    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return false;

    return ircs->IsAutoreconnect(window);
}

void GrumpydSession::SetAutoreconnect(Scrollback *window, bool reconnect)
{
    if (window == this->systemWindow)
        this->AutoReconnect = reconnect;

    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return;

    return ircs->SetAutoreconnect(window, reconnect);
}

libircclient::User *GrumpydSession::GetSelfNetworkID(Scrollback *window)
{
    IRCSession *ircs = this->GetSessionFromWindow(window);
    if (!ircs)
        return NULL;

    return ircs->GetSelfNetworkID(window);
}

unsigned long long GrumpydSession::GetCompressedBytesRcvd()
{
    return this->gp->GetCompBytesRcvd();
}

unsigned long long GrumpydSession::GetCompressedBytesSent()
{
    return this->gp->GetCompBytesSent();
}

unsigned long long GrumpydSession::GetBytesRcvd()
{
    return this->gp->GetBytesRcvd();
}

unsigned long long GrumpydSession::GetBytesSent()
{
    return this->gp->GetBytesSent();
}

unsigned long long GrumpydSession::GetPacketsSent()
{
    return this->gp->GetPacketsSent();
}

unsigned long long GrumpydSession::GetPacketsRcvd()
{
    return this->gp->GetPacketsRecv();
}

bool GrumpydSession::IsReceivingLargePacket()
{
    return this->gp->IsReceiving() && (this->gp->GetIncomingPacketSize() > 2000);
}

qint64 GrumpydSession::GetReceivingPacketSize()
{
    return this->gp->GetIncomingPacketSize();
}

qint64 GrumpydSession::GetProgress()
{
    return this->gp->GetIncomingPacketRecv();
}

QDateTime GrumpydSession::GetLastUpdateOfUserList()
{
    return this->lastUserListUpdate;
}

QList<QVariant> GrumpydSession::GetUserList()
{
    return this->userList;
}

QList<QString> GrumpydSession::GetRoles()
{
    return this->roles;
}

void GrumpydSession::OnSslHandshakeFailure(QList<QSslError> errors, bool *ok)
{
    foreach(QSslError x, errors)
        GRUMPY_ERROR("SSL warning: " + x.errorString());
    *ok = true;
}

void GrumpydSession::OnDisconnect()
{
    this->closeError("Remote host closed the socket!");
}

void GrumpydSession::OnTimeout()
{
    this->closeError("Timed out");
}

void GrumpydSession::OnConnected()
{
    this->systemWindow->InsertText("Connected to remote server, sending HELLO packet");
    QHash<QString, QVariant> parameters;
    parameters.insert("version", QString(GRUMPY_VERSION_STRING));
    this->gp->SendProtocolCommand(GP_CMD_HELLO, parameters);
}

void GrumpydSession::OnError(QString reason, int num)
{
    this->closeError(reason);
}

void GrumpydSession::OnIncomingCommand(gp_command_t text, QHash<QString, QVariant> parameters)
{
    if (text == GP_CMD_UNKNOWN)
    {
        if (parameters.contains("unrecognized"))
            this->systemWindow->InsertText(QString("Grumpyd didn't recognize this command: ") + parameters["unrecognized"].toString(), ScrollbackItemType_SystemError);
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
        bool initial_setup = parameters.contains("initial_setup") && parameters["initial_setup"].toBool();
        if (authentication_required && this->username.isEmpty())
        {
            this->closeError("Remote require authentication, but you didn't provide any credentials needed to login");
            return;
        }
        this->systemWindow->InsertText("Received HELLO from remote system, version of server is: " + parameters["version"].toString());
        if (initial_setup)
        {
            this->systemWindow->InsertText("Remote server requires an initial setup, registering current user and setting up");
            QHash<QString, QVariant> init;
            init.insert("username", this->username);
            init.insert("password", this->password);
            this->SendProtocolCommand(GP_CMD_INIT, init);
            return;
        }
        QHash<QString, QVariant> params;
        params.insert("password", this->password);
        params.insert("username", this->username);
        this->gp->SendProtocolCommand(GP_CMD_LOGIN, params);
    } else if (text == GP_CMD_LOGIN_FAIL)
    {
        emit this->Event_AuthenticationFailed();
        this->AutoReconnect = false;
        this->closeError("Invalid username or password provided");
    } else if (text == GP_CMD_CHANNEL_RESYNC)
    {
        this->processChannelResync(parameters);
    } else if (text == GP_CMD_SCROLLBACK_RESYNC)
    {
        this->processSResync(parameters);
    } else if (text == GP_CMD_REMOVE)
    {
        this->processRemove(parameters);
    } else if (text == GP_CMD_NICK)
    {
        this->processNick(parameters);
    } else if (text == GP_CMD_NETWORK_RESYNC)
    {
        this->processNetworkResync(parameters);
    } else if (text == GP_CMD_SCROLLBACK_PARTIAL_RESYNC)
    {
        this->processPSResync(parameters);
    } else if (text == GP_CMD_CHANNEL_JOIN)
    {
        this->processChannel(parameters);
    } else if (text == GP_CMD_LOGIN_OK)
    {
        this->syncing = true;
        if (parameters.contains("logged"))
        {
            this->systemWindow->InsertText("There is " + QString::number(parameters["logged"].toInt()) + " open sessions as this user");
        }
        this->systemWindow->InsertText("Synchronizing networks");
        this->gp->SendProtocolCommand(GP_CMD_NETWORK_INFO);
        this->syncInit = QDateTime::currentDateTime();
        this->gp->SendProtocolCommand(GP_CMD_OPTIONS);
    } else if (text == GP_CMD_SCROLLBACK_LOAD_NEW_ITEM)
    {
        this->processNewScrollbackItem(parameters);
    } else if (text == GP_CMD_USERLIST_SYNC)
    {
        this->processULSync(parameters);
    } else if (text == GP_CMD_RESYNC_MODE)
    {
        this->processChannelModeSync(parameters);
    } else if (text == GP_CMD_INIT)
    {
        QHash<QString, QVariant> params;
        params.insert("password", this->password);
        params.insert("username", this->username);
        this->gp->SendProtocolCommand(GP_CMD_LOGIN, params);
    } else if (text == GP_CMD_NETWORK_INFO)
    {
        this->processNetwork(parameters);
    } else if (text == GP_CMD_REQUEST_ITEMS)
    {
        this->processRequest(parameters);
    } else if (text == GP_CMD_IRC_QUIT)
    {
        IRCSession *session = this->GetSession(parameters["network_id"].toUInt());
        if (!session)
            return;

        // Let's flag everything disconnected
        foreach (Scrollback *sb, session->GetScrollbacks())
            sb->SetDead(true);
    } else if (text == GP_CMD_PERMDENY)
    {
        QString source = "unknown request";
        if (parameters.contains("source"))
            source = parameters["source"].toString();
        this->systemWindow->InsertText("Permission denied: " + source, ScrollbackItemType_SystemError);
    } else if (text == GP_CMD_ERROR)
    {
        this->systemWindow->InsertText("Error: " + parameters["description"].toString(), ScrollbackItemType_SystemError);
    } else if (text == GP_CMD_OPTIONS)
    {
        this->processPreferences(parameters);
    } else if (text == GP_CMD_RESYNC_SCROLLBACK_PB)
    {
        this->processPBResync(parameters);
    } else if (text == GP_CMD_SYS_LIST_USER)
    {
        this->processUserList(parameters);
    } else if (text == GP_CMD_SYS_CREATE_USER)
    {
        if (parameters.contains("username"))
        {
            this->systemWindow->InsertText("Successfuly added user: " + parameters["username"].toString());
        }
    } else if (text == GP_CMD_SYS_REMOVE_USER)
    {
        if (parameters.contains("username"))
        {
            this->systemWindow->InsertText("Removed user: " + parameters["username"].toString());
        }
    } else if (text == GP_CMD_SYS_LOCK_USER)
    {
        if (parameters.contains("username"))
        {
            this->systemWindow->InsertText("Successfuly locked user: " + parameters["username"].toString());
        }
    } else if (text == GP_CMD_SYS_UNLOCK_USER)
    {
        if (parameters.contains("username"))
        {
            this->systemWindow->InsertText("Successfuly unlocked user: " + parameters["username"].toString());
        }
    } else
    {
        QHash<QString, QVariant> params;
        params.insert("source", text);
        this->gp->SendProtocolCommand(GP_CMD_UNKNOWN, params);
        this->systemWindow->InsertText("Unknown command from grumpyd " + QString::number(text), ScrollbackItemType_SystemError);
    }
}

void GrumpydSession::kill()
{
    this->systemWindow->SetDead(true);
    // flag every scrollback as dead
    foreach (IRCSession *session, this->sessionList.values())
    {
        foreach (Scrollback *window, session->GetScrollbacks())
            window->SetDead(true);
    }
    this->scrollbackHash.clear();
    this->roles.clear();
    this->userList.clear();
}

void GrumpydSession::processNewScrollbackItem(QHash<QString, QVariant> hash)
{
    //if (!hash.contains("network_id"))
    //    return;
    // Fetch the network this item belongs to
    //unsigned int sid = hash["network_id"].toUInt();
    if (!hash.contains("scrollback"))
    {
        GRUMPY_DEBUG("Missing scrollback id for item", 2);
        return;
    }
    scrollback_id_t id = hash["scrollback"].toUInt();
    Scrollback *window = this->GetScrollback(id);
    if (!window)
    {
        GRUMPY_DEBUG("Received scrollback item for scrollback which couldn't be found, name: " + hash["scrollback_name"].toString(), 3);
        return;
    }
    window->InsertText(ScrollbackItem(hash["item"].toHash()));
}

void GrumpydSession::processNetwork(QHash<QString, QVariant> hash)
{
    if (!hash.contains("sessions"))
        return;

    this->IsOpening = true;
    // Deserialize all irc sessions
    QList<QVariant> session_list = hash["sessions"].toList();
    foreach (QVariant session_hash, session_list)
    {
        IRCSession *session = new IRCSession(session_hash.toHash(), this->systemWindow);
        session->GetSystemWindow()->SetSession(this);
        this->sessionList.insert(session->GetSystemWindow(), session);
    }
    if (this->syncing)
    {
        this->systemWindow->InsertText("Networks were synced, " + QString::number(session_list.count()) +
                                       " networks synced in " +
                                       QString::number(this->syncInit.secsTo(QDateTime::currentDateTime())) +
                                       " seconds.");
        this->syncing = false;
    }
    this->IsOpening = false;
}

void GrumpydSession::processULSync(QHash<QString, QVariant> hash)
{
    QString channel_name = hash["channel_name"].toString();
    IRCSession *session = this->GetSession(hash["network_id"].toUInt());
    if (!session)
        return;
    libircclient::Channel *channel = session->GetNetwork()->GetChannel(channel_name);
    int mode = hash["operation"].toInt();
    if (!channel || !mode)
        return;
    Scrollback *window = session->GetScrollback(channel_name);
    if (!window)
    {
        GRUMPY_DEBUG("Unable to resync user list, window is missing for " + channel_name, 1);
        return;
    }
    if (mode == GRUMPY_UL_INSERT)
    {
        libircclient::User user(hash["user"].toHash());
        channel->InsertUser(&user);
        window->UserListChange(user.GetNick(), &user, UserListChange_Insert);
    } else if (mode == GRUMPY_UL_REMOVE)
    {
        QString user = hash["target"].toString();
        // Remove the user from window
        window->UserListChange(user, NULL, UserListChange_Remove);
        channel->RemoveUser(user);
    } else if (mode == GRUMPY_UL_UPDATE)
    {
        libircclient::User user(hash["user"].toHash());
        channel->InsertUser(&user);
        window->UserListChange(user.GetNick(), &user, UserListChange_Refresh);
    }
}

void GrumpydSession::processNetworkResync(QHash<QString, QVariant> hash)
{
    if (!hash.contains("network_id") || !hash.contains("network"))
        return;
    IRCSession *session = this->GetSession(hash["network_id"].toUInt());
    if (!session)
        return;

    bool is_partial = hash.contains("partial") && hash["partial"].toBool();

    if (is_partial)
    {
        // This is just a partial update of existing network
        session->GetNetwork()->LoadHash(hash["network"].toHash());
        return;
    }

    libircclient::Network resynced_network(hash["network"].toHash());
    libircclient::Network *nt = session->GetNetwork();
    if (nt == NULL)
    {
        libirc::ServerAddress server(session->GetHostname(), session->UsingSSL(), session->GetPort(), session->GetNick(), session->GetPassword());
        nt = new libircclient::Network(server, session->GetName());
        session->SetNetwork(nt);
    }
    nt->SetCUModes(resynced_network.GetCUModes());
    nt->SetCCModes(resynced_network.GetCCModes());
    nt->SetChannelUserPrefixes(resynced_network.GetChannelUserPrefixes());
    nt->SetCModes(resynced_network.GetCModes());
    nt->SetCPModes(resynced_network.GetCPModes());
    nt->SetCRModes(resynced_network.GetCRModes());
    nt->SetCUModes(resynced_network.GetCUModes());
}

void GrumpydSession::processChannel(QHash<QString, QVariant> hash)
{
    IRCSession *session = this->GetSession(hash["network_id"].toUInt());
    if (!session)
        return;
    libircclient::Channel channel(hash["channel"].toHash());
    Scrollback *window = this->GetScrollback(hash["scrollback_id"].toUInt());
    if (!window)
        return;
    session->RegisterChannel(&channel, window);
}

void GrumpydSession::processNick(QHash<QString, QVariant> hash)
{
    IRCSession *session = this->GetSession(hash["network_id"].toUInt());
    if (!session)
        return;
    QString new_ = hash["new_nick"].toString();
    QString old_ = hash["old_nick"].toString();
    // Check if the nick is our own nickname in first place
    if (session->GetNetwork()->GetNick().toLower() == old_.toLower())
    {
        // yup, it's our so we need to update it
        session->GetNetwork()->_st_SetNick(new_);
    }
    session->_gs_ResyncNickChange(new_, old_);
}

void GrumpydSession::processPreferences(QHash<QString, QVariant> hash)
{
    this->Preferences = Generic::MergeHash(this->Preferences, hash["options"].toHash());
    // Automatically fix some missing default preferences
    bool is_missing = false;
    if (!this->Preferences.contains("offline_ms_bool"))
    {
        is_missing = true;
        this->Preferences.insert("offline_ms_bool", true);
    }
    if (!this->Preferences.contains("offline_ms_text"))
    {
        is_missing = true;
        this->Preferences.insert("offline_ms_text", "I am currently away, your message was logged and I will read it when I return");
    }
    if (!this->Preferences.contains("maximum_bsize"))
    {
        is_missing = true;
        this->Preferences.insert("maximum_bsize", 2000);
    }
    if (!this->Preferences.contains("nick"))
    {
        is_missing = true;
        this->Preferences.insert("nick", Core::GrumpyCore->GetConfiguration()->GetValueAsString("nick", "GrumpydUser"));
    }
    if (!this->Preferences.contains("quit_message"))
    {
        is_missing = true;
        this->Preferences.insert("quit_message", "GrumpyChat - bouncer shutting down for maintenance. https://github.com/grumpy-irc/grumpy");
    }
    if (is_missing)
    {
        this->systemWindow->InsertText("No default user preferences found for this user, fixing up");
        QHash<QString, QVariant> values;
        values.insert("merge", this->Preferences);
        this->SendProtocolCommand(GP_CMD_OPTIONS, values);
    }
}

void GrumpydSession::processChannelModeSync(QHash<QString, QVariant> hash)
{
    IRCSession *session = this->GetSession(hash["network_id"].toUInt());
    if (!session)
        return;
    libircclient::Channel *channel = session->GetNetwork()->GetChannel(hash["channel_name"].toString());
    if (!channel)
        return;
    int mt = hash["type"].toInt();
    if (mt == GP_MODETYPE_PMODE)
    {
        libircclient::ChannelPMode mode(hash["mode"].toHash());
        QString type = hash["operation"].toString();
        if (type == "insert")
            channel->SetPMode(mode);
        else if (type == "remove")
            channel->RemovePMode(mode);
    }
}

void GrumpydSession::processRequest(QHash<QString, QVariant> hash)
{
    Scrollback *scrollback = this->GetScrollback(hash["scrollback_id"].toUInt());
    if (!scrollback)
        return;

    this->IsOpening = true;
    QList<QVariant> lx = hash["data"].toList();
    QList<ScrollbackItem> data;
    foreach (QVariant item, lx)
        data.append(ScrollbackItem(item.toHash()));
    scrollback->PrependItems(data);
    this->IsOpening = false;
}

void GrumpydSession::processChannelResync(QHash<QString, QVariant> hash)
{
    IRCSession *session = this->GetSession(hash["network_id"].toUInt());
    if (!session)
        return;
    libircclient::Channel *channel = session->GetNetwork()->GetChannel(hash["channel_name"].toString());
    if (hash.contains("partial") && hash["partial"].toBool())
    {
        if (channel)
            channel->LoadHash(hash["channel"].toHash());

        // That's all :)
        return;
    }
    libircclient::Channel resynced_channel(hash["channel"].toHash());
    // Find a scrollback that is associated to this channel if there is some
    Scrollback *window = session->GetScrollback(resynced_channel.GetName());
    if (!channel)
    {
        // There is no such a channel, so let's insert it to network structure
        channel = session->GetNetwork()->_st_InsertChannel(&resynced_channel);
        // Resync the users with scrollback and quit
        foreach (libircclient::User *user, channel->GetUsers())
        {
            if (window)
                window->UserListChange(user->GetNick(), user, UserListChange_Insert, true);
        }
        window->FinishBulk();
        return;
    }

    if (window)
    {
        // Remove all users from the window's internal user list
        foreach (libircclient::User *user, channel->GetUsers())
            window->UserListChange(user->GetNick(), user, UserListChange_Remove, true);
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
            window->UserListChange(user->GetNick(), user, UserListChange_Insert, true);
    }
    window->FinishBulk();
}

void GrumpydSession::processSResync(QHash<QString, QVariant> parameters)
{
    // Let's register this scrollback to this session
    Scrollback *root = this->systemWindow;
    scrollback_id_t parent = this->systemWindow->GetID();
    if (parameters.contains("parent_sid"))
    {
        parent = parameters["parent_sid"].toUInt();
        root = this->GetScrollback(parent);
        if (!root)
            root = this->systemWindow;
    }
    Scrollback *result = this->GetScrollback(parameters["scrollback"].toHash()["_original_id"].toUInt());
    if (result)
    {
        // Restore the original
        result->LoadHash(parameters["scrollback"].toHash());
    } else
    {
        result = Core::GrumpyCore->NewScrollback(root, parameters["name"].toString(), ScrollbackType_User);
        result->SetSession(this);
        result->LoadHash(parameters["scrollback"].toHash());
        this->scrollbackHash.insert(result->GetOriginalID(), result);
    }
    // Recover the lists
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

void GrumpydSession::processPBResync(QHash<QString, QVariant> parameters)
{
    if (!parameters.contains("scrollback_id"))
        return;
    Scrollback *scrollback = this->GetScrollback(parameters["scrollback_id"].toUInt());
    if (!scrollback)
        return;
    if (!parameters.contains("property_bag"))
        return;
    QHash<QString, QVariant> pb = parameters["property_bag"].toHash();
    foreach(QString key, pb.keys())
    {
        if (scrollback->PropertyBag.contains(key))
            scrollback->PropertyBag[key] = pb[key];
        else
            scrollback->PropertyBag.insert(key, pb[key]);
    }
}

void GrumpydSession::processAck(QHash<QString, QVariant> parameters)
{
    if (parameters.contains("s"))
    {
        unsigned int id = parameters["s"].toString().toUInt();
        if (this->processedMessages.contains(id))
            this->processedMessages.removeOne(id);
    }
}

void GrumpydSession::processPSResync(QHash<QString, QVariant> parameters)
{
    //Scrollback scrollback(parameters["scrollback"].toHash());
    // find a scrollback with this id
    QHash<QString, QVariant> scrollback = parameters["scrollback"].toHash();
    if (!scrollback.contains("_original_id"))
    {
        this->systemWindow->InsertText("RESYNC ERROR: Failed to resync scrollback, missing original_id", ScrollbackItemType_SystemError);
        return;
    }
    Scrollback *origin = this->GetScrollback(scrollback["_original_id"].toUInt());
    if (!origin)
    {
        this->systemWindow->InsertText("RESYNC ERROR: Failed to resync scrollback with id " + QString::number(scrollback["_original_id"].toUInt()), ScrollbackItemType_SystemError);
        return;
    }
    // let's resync most of the stuff
    origin->LoadHash(parameters["scrollback"].toHash());
    origin->Resync(NULL);
}

void GrumpydSession::processRemove(QHash<QString, QVariant> parameters)
{
    if (!parameters.contains("scrollback"))
        return;
    Scrollback *scrollback = this->GetScrollback(parameters["scrollback"].toUInt());
    if (!scrollback)
        return;
    if (scrollback == this->systemWindow)
        return;
    IRCSession *session = this->GetSessionFromWindow(scrollback);
    if (!session)
        return;
    if (!scrollback->IsDead() && scrollback->GetType() != ScrollbackType_User)
    {
        this->closeError("Server requested illegal operation: removal of a window which is being used");
        return;
    }
    if (session->GetSystemWindow() == scrollback)
    {
        // We remove whole network here
        if (!this->sessionList.contains(scrollback))
            throw new GrumpyIRC::Exception("Hash table is not consistent with server", BOOST_CURRENT_FUNCTION);

        this->sessionList.remove(scrollback);
        session->RequestRemove(scrollback);
        delete session;
        return;
    }
    session->RequestRemove(scrollback);
}

void GrumpydSession::processUserList(QHash<QString, QVariant> parameters)
{
    this->lastUserListUpdate = QDateTime::currentDateTime();
    if (parameters.contains("roles"))
    {
        QList<QVariant> roles = parameters["roles"].toList();
        this->roles.clear();
        foreach (QVariant role, roles)
            this->roles.append(role.toString());
    }
    if (parameters.contains("list"))
    {
        this->userList = parameters["list"].toList();
    }
}

void GrumpydSession::freememory()
{

}

void GrumpydSession::closeError(QString error)
{
    this->gp->Disconnect();
    this->kill();
    this->systemWindow->InsertText("Connection failure: " + error, ScrollbackItemType_SystemError);
}

