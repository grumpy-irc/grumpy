//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QHostAddress>
#include <QHash>
#include <QSslSocket>
#include "../libirc/libirc/serveraddress.h"
#include "../libirc/libircclient/network.h"
#include "../libcore/core.h"
#include "../libcore/grumpydsession.h"
#include "../libcore/eventhandler.h"
#include "../libcore/generic.h"
#include "databasebackend.h"
#include "grumpyconf.h"
#include "corewrapper.h"
#include "userconfiguration.h"
#include "grumpyd.h"
#include "user.h"
#include "security.h"
#include "session.h"
#include "syncableircsession.h"

using namespace GrumpyIRC;

QMutex *Session::sessions_lock = new QMutex();
QList<Session*> Session::SessionList;
unsigned long Session::lSID = 0;

QList<Session *> Session::Sessions()
{
    return SessionList;
}

Session::Session(qintptr socket_ptr, bool ssl)
{
    this->usingSsl = ssl;
    this->protocol = NULL;
    if (ssl)
    {
        QSslSocket *ssl_socket = new QSslSocket();
        ssl_socket->setLocalCertificate(Grumpyd::GetPathSSLCert());
        ssl_socket->setPrivateKey(Grumpyd::GetPathSSLKey());
        this->socket = ssl_socket;
    }
    else
    {
        this->socket = new QTcpSocket();
    }
    if (!this->socket->setSocketDescriptor(socket_ptr))
    {
        GRUMPY_ERROR("Unable to set socket descriptor " + QString::number(socket_ptr) + " for new session");
        goto failure;
    }
    this->peer = this->socket->peerAddress().toString();
    sessions_lock->lock();
    this->SID = lSID++;
    this->IsRunning = true;
    SessionList.append(this);
    sessions_lock->unlock();
    this->MaxScrollbackSyncItems = 80;
    this->loggedUser = NULL;
    if (ssl)
    {
        // setup handshake
        QSslSocket *ssl_socket = (QSslSocket*)this->socket;
        ssl_socket->startServerEncryption();
        if (!ssl_socket->waitForEncrypted())
        {
            GRUMPY_ERROR("SSL handshake failed for SID " + QString::number(this->GetSID()) + " peer: "
                         + this->peer + ": " + ssl_socket->errorString());
            goto failure;
        }
    }
    this->protocol = new libgp::GP(socket);
    this->protocol->SetCompression(6);
    this->SessionState = State_Login;
    connect(this->protocol, SIGNAL(Event_IncomingCommand(gp_command_t,QHash<QString,QVariant>)), this, SLOT(OnCommand(gp_command_t,QHash<QString,QVariant>)));
    connect(this->protocol, SIGNAL(Event_Disconnected()), this, SLOT(OnDisconnected()));
    this->protocol->ResolveSignals();
    GRUMPY_LOG("New session (" + QString::number(this->SID) + ") from " + this->peer);
    return;

    failure:
        this->SessionState = State_Offline;
        this->socket->close();
}

Session::~Session()
{
    sessions_lock->lock();
    SessionList.removeOne(this);
    sessions_lock->unlock();
    // deletion of socket is performed by destructor of protocol
    GRUMPY_LOG("Session for " + this->peer + " destroyed");
    delete this->protocol;
    if (this->loggedUser)
    {
        // Remove the session from list of sessions this user has open
        this->loggedUser->RemoveSession(this);
    }
}

void Session::run()
{
    if (this->SessionState == State_Offline)
    {
        this->deleteLater();
        return;
    }
    while(this->IsRunning)
        sleep(1);

    // exit the session
    this->SessionState = State_Offline;
    this->deleteLater();
}

unsigned long Session::GetSID()
{
    return this->SID;
}

bool Session::IsAuthorized(QString permission)
{
    if (!this->loggedUser)
        return false;

    return this->loggedUser->IsAuthorized(permission);
}

void Session::SendToEverySession(gp_command_t command, QHash<QString, QVariant> parameters)
{
    if (!this->loggedUser)
        return;

    foreach (Session *xx, this->loggedUser->GetGPSessions())
        xx->protocol->SendProtocolCommand(command, parameters);
}

void Session::SendToOtherSessions(gp_command_t command, QHash<QString, QVariant> parameters)
{
    if (!this->loggedUser)
        return;

    foreach (Session *xx, this->loggedUser->GetGPSessions())
    {
        if (xx != this)
        {
            xx->protocol->SendProtocolCommand(command, parameters);
        }
    }
}

Scrollback *Session::GetScrollback(scrollback_id_t scrollback_id)
{
    Scrollback *result = NULL;
    if (!this->loggedUser)
        return NULL;
    foreach (SyncableIRCSession *session, this->loggedUser->GetSIRCSessions())
    {
        result = session->GetScrollback(scrollback_id);
        if (result)
            return result;
    }
    return NULL;
}

void Session::TransferError(gp_command_t source, QString description, int id)
{
    QHash<QString, QVariant> params;
    params.insert("id", QVariant(id));
    params.insert("description", QVariant(description));
    params.insert("source", QVariant(source));
    this->protocol->SendProtocolCommand(GP_CMD_ERROR, params);
}

void Session::PermissionDeny(gp_command_t source)
{
    QHash<QString, QVariant> params;
    params.insert("source", QVariant(source));
    this->protocol->SendProtocolCommand(GP_CMD_PERMDENY, params);
}

void Session::OnDisconnected()
{
    this->SessionState = State_Exiting;
    this->IsRunning = false;
}

void Session::processNetworks()
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_NETWORK_INFO);
        return;
    }

    // Send list of serialized irc sessions
    QList<QVariant> sessions;
    QList<SyncableIRCSession*> network_info = this->loggedUser->GetSIRCSessions();
    foreach (SyncableIRCSession *session, network_info)
        sessions.append(QVariant(session->ToHash(this->MaxScrollbackSyncItems)));
    QHash<QString, QVariant> params;
    params.insert("sessions", sessions);
    this->protocol->SendProtocolCommand(GP_CMD_NETWORK_INFO, params);
}

void Session::processIrcQuit(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_IRC_QUIT);
        return;
    }

    unsigned int nsid = parameters["network_id"].toUInt();
    SyncableIRCSession* irc = this->loggedUser->GetSIRCSession(nsid);
    if (!irc)
    {
        GRUMPY_ERROR("Unable to find network with id " + QString::number(nsid) + " network can't be disconnected for user " + this->loggedUser->GetName());
        this->TransferError(GP_CMD_IRC_QUIT, "Network not found", GP_ENETWORKNOTFOUND);
        return;
    }
    irc->RequestDisconnect(NULL, parameters["reason"].toString(), false);
    this->SendToEverySession(GP_CMD_IRC_QUIT, parameters);
}

void Session::processMessage(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_MESSAGE);
        return;
    }

    // We have the original ID of scrollback so let's find it here
    unsigned int nsid = parameters["network_id"].toUInt();
    scrollback_id_t sid = parameters["scrollback_id"].toUInt();
    QString target;
    if (parameters.contains("target"))
        target = parameters["target"].toString();
    // let's find the network
    SyncableIRCSession* irc = this->loggedUser->GetSIRCSession(nsid);
    if (!irc)
    {
        GRUMPY_ERROR("Unable to find network with id " + QString::number(nsid) + " message can't be delivered for user " + this->loggedUser->GetName());
        this->TransferError(GP_CMD_MESSAGE, "Network not found", GP_ENETWORKNOTFOUND);
        return;
    }
    Scrollback *scrollback = irc->GetScrollback(sid);
    if (!scrollback)
    {
        GRUMPY_ERROR("Unable to find scrollback id " + QString::number(sid) + " message can't be delivered for user " + this->loggedUser->GetName());
        return;
    }
    if (!parameters.contains("type"))
    {
        this->TransferError(GP_CMD_MESSAGE, "Unknown message type", GP_ERROR);
        return;
    }
    int type = parameters["type"].toInt();
    QString text = parameters["text"].toString();
    switch (type)
    {
        case GP_MESSAGETYPE_ACTION:
        {
            irc->SendAction(scrollback, text);
        }
            break;
        case GP_MESSAGETYPE_ISCTCP:
        {
            irc->SendCTCP(scrollback, target, parameters["name"].toString(), text);
        }
            break;
        case GP_MESSAGETYPE_NORMAL:
        {
            if (target.isEmpty())
                irc->SendMessage(scrollback, text);
            else
                irc->SendMessage(scrollback, target, text);
        }
            break;

        case GP_MESSAGETYPE_NOTICE:
        {
            if (target.isEmpty())
                irc->SendNotice(scrollback, text);
            else
                irc->SendNotice(scrollback, target, text);
        }
            return;
        default:
            this->TransferError(GP_CMD_MESSAGE, "Unsupported message type", GP_ERROR);
            return;
    }
}

void Session::processCommand(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_RAW);
        return;
    }

    unsigned int nsid = parameters["network_id"].toUInt();
    // let's find the network
    SyncableIRCSession* irc = this->loggedUser->GetSIRCSession(nsid);
    if (!irc)
        return;
    QString rx = parameters["command"].toString();
    if (!irc->IsConnected())
    {
        this->TransferError(GP_CMD_RAW, "Not connected", GP_EIRCNOTCONN);
        return;
    }
    irc->GetNetwork()->TransferRaw(rx);
}

void Session::processNew(QHash<QString, QVariant> info)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_SERVER);
        return;
    }

    // User wants to connect to some server
    if (!info.contains("server"))
    {
        this->TransferError(GP_CMD_SERVER, "No server host provided", GP_ENOSERVER);
        return;
    }

    libirc::ServerAddress server(info["server"].toHash());
    SyncableIRCSession *session = this->loggedUser->ConnectToIRCServer(server);
    // now we need to deliver message to every session that new connection to a server was open
    QList<QVariant> network_info;
    QHash<QString, QVariant> parameters;
    network_info.append(QVariant(session->ToHash()));
    parameters.insert("sessions", network_info);
    this->SendToEverySession(GP_CMD_NETWORK_INFO, parameters);
}

void Session::processInfo(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_REQUEST_INFO);
        return;
    }

    if (!parameters.contains("network_id"))
    {
        this->TransferError(GP_CMD_REQUEST_INFO, "No network provided", GP_ENOSERVER);
        return;
    }

    SyncableIRCSession* irc = this->loggedUser->GetSIRCSession(parameters["network_id"].toUInt());
    if (!irc)
    {
        this->TransferError(GP_CMD_REQUEST_INFO, "Network not found", GP_ENETWORKNOTFOUND);
        return;
    }
    if (!parameters.contains("type"))
        return;
    QString tx = parameters["type"].toString();
    if (tx == "+b")
        irc->RetrieveChannelBanList(NULL, parameters["channel_name"].toString());
    else if (tx == "+I")
        irc->RetrieveChannelInviteList(NULL, parameters["channel_name"].toString());
    else if (tx == "+e")
        irc->RetrieveChannelExceptionList(NULL, parameters["channel_name"].toString());
}

void Session::processUserList(QHash<QString, QVariant> parameters)
{
    Q_UNUSED(parameters);
    if (!this->IsAuthorized(PRIVILEGE_LIST_USERS))
    {
        this->PermissionDeny(GP_CMD_SYS_LIST_USER);
        return;
    }

    // Create user list
    QList<QVariant> user_list;
    foreach (User *user, User::UserInfo)
    {
        QHash<QString, QVariant> user_data;
        user_data.insert("name", user->GetName());
        user_data.insert("id", user->GetID());
        user_data.insert("role", user->GetRole()->GetName());
        user_data.insert("online", user->IsOnline());
        user_data.insert("irc_session_count", user->GetIRCSessionCount());
        user_data.insert("gp_session_count", user->GetSessionCount());
        user_data.insert("locked", user->IsLocked());
        user_list.append(QVariant(user_data));
    }

    QHash<QString, QVariant> response;
    response.insert("list", QVariant(user_list));

    // Roles as well
    QList<QVariant> role_list;
    foreach (Role *role, Role::Roles)
            role_list.append(role->GetName());
    response.insert("roles", QVariant(role_list));

    this->protocol->SendProtocolCommand(GP_CMD_SYS_LIST_USER, response);
}

void Session::OnCommand(gp_command_t text, QHash<QString, QVariant> parameters)
{
    if (text == GP_CMD_HELLO)
    {
        if (parameters.contains("version"))
            GRUMPY_DEBUG(QString("SID ") + QString::number(this->SID) + QString(" version ") + parameters["version"].toString(), 1);
        // respond to HELLO which is a first command that is meant to be sent by a client to server and only server
        // can respond to it
        QHash<QString, QVariant> params;
        params.insert("version", QVariant(QString("Grumpyd ") + GRUMPY_VERSION_STRING));
        if (CONF->Init)
            params.insert("initial_setup", true);
        params.insert("authentication_required", QVariant(!CONF->Init));
        this->protocol->SendProtocolCommand(GP_CMD_HELLO, params);
    } else if (text == GP_CMD_UNKNOWN)
    {
        if (!parameters.contains("unrecognized"))
            return;
        GRUMPY_DEBUG(QString("SID ") + QString::number(this->SID) + QString(" doesn't understand protocol ") + parameters["unrecognized"].toString(), 2);
    } else if (text == GP_CMD_RAW)
    {
        this->processCommand(parameters);
    } else if (text == GP_CMD_LOGIN)
    {
        this->processLogin(parameters);
    } else if (text == GP_CMD_IRC_QUIT)
    {
        this->processIrcQuit(parameters);
    } else if (text == GP_CMD_MESSAGE)
    {
        this->processMessage(parameters);
    } else if (text == GP_CMD_NETWORK_INFO)
    {
        this->processNetworks();
    } else if (text == GP_CMD_SERVER)
    {
        this->processNew(parameters);
    } else if (text == GP_CMD_RECONNECT)
    {
        this->processReconnect(parameters);
    } else if (text == GP_CMD_SCROLLBACK_PARTIAL_RESYNC)
    {
        this->processResync(parameters);
    } else if (text == GP_CMD_REQUEST_ITEMS)
    {
        this->processRequest(parameters);
    } else if (text == GP_CMD_REQUEST_INFO)
    {
        this->processInfo(parameters);
    } else if (text == GP_CMD_REMOVE)
    {
        this->processRemove(parameters);
    } else if (text == GP_CMD_INIT)
    {
        this->processSetup(parameters);
    } else if (text == GP_CMD_OPTIONS)
    {
        this->processOptions(parameters);
    } else if (text == GP_CMD_SYS_LIST_USER)
    {
        this->processUserList(parameters);
    } else if (text == GP_CMD_RESYNC_SCROLLBACK_PB)
    {
        this->processPBResync(parameters);
    } else
    {
        // We received some unknown packet, send it back to client so that it at least knows we don't support this
        QHash<QString, QVariant> params;
        params.insert("unrecognized", QVariant(text));
        this->protocol->SendProtocolCommand(GP_CMD_UNKNOWN, params);
    }
}

void Session::processLogin(QHash<QString, QVariant> parameters)
{
    // User wants to login
    if (this->loggedUser)
    {
        this->TransferError(GP_CMD_LOGIN, "You are already logged in", GP_EALREADYLOGGEDIN);
        return;
    }
    if (!parameters.contains("username") || !parameters.contains("password"))
    {
        this->TransferError(GP_CMD_LOGIN, "Invalid parameters", GP_EINVALIDLOGINPARAMS);
        return;
    }
    this->loggedUser = User::Login(parameters["username"].toString(), User::EncryptPw(parameters["password"].toString()));
    if (this->loggedUser)
    {
        if (!this->IsAuthorized(PRIVILEGE_LOGIN))
        {
            this->PermissionDeny(GP_CMD_LOGIN);
            return;
        }
        QHash<QString, QVariant> param;
        param.insert("logged", this->loggedUser->GetSessionCount());
        this->protocol->SendProtocolCommand(GP_CMD_LOGIN_OK, param);
        GRUMPY_LOG("SID " + QString::number(this->SID) + " identified to " + this->loggedUser->GetName());
        this->loggedUser->InsertSession(this);
    } else
    {
        GRUMPY_LOG("SID " + QString::number(this->SID) + " failed to login to name " + parameters["username"].toString());
        this->protocol->SendProtocolCommand(GP_CMD_LOGIN_FAIL);
    }
}

void Session::processRequest(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_REQUEST_ITEMS);
        return;
    }

    scrollback_id_t from = parameters["from"].toUInt();
    unsigned int size = parameters["request_size"].toUInt();
    if (size > CONF->GetMaxLoadSize())
        size = CONF->GetMaxLoadSize();
    scrollback_id_t sx = parameters["scrollback_id"].toUInt();
    Scrollback *scrollback = this->GetScrollback(sx);
    if (!scrollback)
    {
        GRUMPY_ERROR("Unable to find scrollback " + QString::number(sx) + " for user " + this->loggedUser->GetName());
        this->TransferError(GP_CMD_REQUEST_ITEMS, "dScrollback not found", GP_ESCROLLBACKNOTFOUND);
        return;
    }
    QVariant list;
    try
    {
        list = QVariant(scrollback->FetchBacklog(from, size));
    } catch (GrumpyIRC::Exception *ex)
    {
        this->TransferError(GP_CMD_REQUEST_ITEMS, "Exception: " + ex->GetMessage(), GP_ERROR);
        delete ex;
    }
    QHash<QString, QVariant> response;
    response.insert("data", list);
    response.insert("scrollback_id", QVariant(sx));
    this->protocol->SendProtocolCommand(GP_CMD_REQUEST_ITEMS, response);
}

void Session::processSetup(QHash<QString, QVariant> parameters)
{
    if (!CONF->Init)
    {
        this->PermissionDeny(GP_CMD_INIT);
        return;
    }

    if (!parameters.contains("username") || !parameters.contains("password"))
    {
        this->TransferError(GP_CMD_INIT, "Missing user", 0);
        return;
    }

    // Register a new user account
    User *user = new User(parameters["username"].toString(), User::EncryptPw(parameters["password"].toString()), 0);
    if (!Role::Roles.contains("root"))
        throw new Exception("Built-in root not found", BOOST_CURRENT_FUNCTION);
    user->SetRole(Role::Roles["root"]);
    User::UserInfo.append(user);
    Grumpyd::GetBackend()->StoreUser(user);
    CONF->Init = false;

    QHash<QString, QVariant> result;
    result.insert("done", QVariant(true));
    this->protocol->SendProtocolCommand(GP_CMD_INIT, result);
}

void Session::processResync(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_SCROLLBACK_PARTIAL_RESYNC);
        return;
    }

    QHash<QString, QVariant> scrollback = parameters["scrollback"].toHash();
    if (!scrollback.contains("_original_id"))
        return;
    Scrollback *origin = this->GetScrollback(scrollback["_original_id"].toUInt());
    if (!origin)
    {
        //this->systemWindow->InsertText("RESYNC ERROR: Failed to resync scrollback with id " + QString::number(scrollback["_original_id"].toUInt()), ScrollbackItemType_SystemError);
        return;
    }
    // let's resync most of the stuff
    origin->LoadHash(parameters["scrollback"].toHash());
    // we need to resync this with other clients as well
    // let's just forward this
    this->SendToOtherSessions(GP_CMD_SCROLLBACK_PARTIAL_RESYNC, parameters);
}

void Session::processReconnect(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_RECONNECT);
        return;
    }

    SyncableIRCSession* irc = this->loggedUser->GetSIRCSession(parameters["network_id"].toUInt());
    if (!irc)
    {
        this->TransferError(GP_CMD_RECONNECT, "Network not found :(", GP_ENETWORKNOTFOUND);
        return;
    }

    if (irc->IsConnected())
    {
        this->TransferError(GP_CMD_RECONNECT, "Network is connected", GP_ENETWORKNOTFOUND);
        return;
    }

    irc->Connect();
}

void Session::processOptions(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_OPTIONS);
        return;
    }

    QHash<QString, QVariant> reply;
    if (parameters.contains("get"))
    {
        QHash <QString, QVariant> option;
        foreach (QVariant key, parameters["get"].toList())
        {
            QString kx = key.toString();
            option.insert(kx, this->loggedUser->GetConfiguration()->GetValue(kx));
        }
        reply.insert("options", QVariant(option));
        this->protocol->SendProtocolCommand(GP_CMD_OPTIONS, reply);
    } else if (parameters.contains("set"))
    {
        QHash<QString, QVariant> ox = parameters["set"].toHash();
        foreach (QString key, ox.keys())
            this->loggedUser->GetConfiguration()->SetValue(key, ox[key]);

        this->loggedUser->GetConfiguration()->Save();
    } else if (parameters.contains("override"))
    {
        this->loggedUser->GetConfiguration()->SetHash(parameters["override"].toHash());
        this->loggedUser->GetConfiguration()->Save();
    } else if (parameters.contains("merge"))
    {
        this->loggedUser->GetConfiguration()->SetHash(parameters["merge"].toHash());
        this->loggedUser->GetConfiguration()->Save();
    } else
    {
        reply.insert("options", QVariant(this->loggedUser->GetConfiguration()->ToHash()));
        this->protocol->SendProtocolCommand(GP_CMD_OPTIONS, reply);
    }
}

void Session::processRemove(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_REMOVE);
        return;
    }

    SyncableIRCSession* irc = this->loggedUser->GetSIRCSession(parameters["network_id"].toUInt());
    if (!irc)
    {
        this->TransferError(GP_CMD_REMOVE, "Network not found :(", GP_ENETWORKNOTFOUND);
        return;
    }

    // We need to figure out which window it is, if it's even here
    Scrollback *scrollback = this->GetScrollback(parameters["scrollback_id"].toUInt());
    if (!scrollback)
    {
        this->TransferError(GP_CMD_REMOVE, "Scrollback not found: " + QString::number(parameters["scrollback_id"].toUInt()), GP_ESCROLLBACKNOTFOUND);
        return;
    }

    if (scrollback == irc->GetSystemWindow())
    {
        // User wants to remove whole network from database, first we need to check if it's disconnected and send response if it's not
        if (irc->IsConnected())
        {
            this->TransferError(GP_CMD_REMOVE, "Network you requested to remove is still connected to server", GP_ERROR);
            return;
        }

        // Now remove the network
        irc->RequestRemove(scrollback);
        // Drop the network from user's db
        Grumpyd::GetBackend()->RemoveNetwork(irc);
        this->loggedUser->RemoveIRCSession(irc);
        delete irc;
    } else
    {
        if (!scrollback->IsDead() && scrollback->GetType() != ScrollbackType_User)
        {
            this->TransferError(GP_CMD_REMOVE, "You can't remove scrollback that is still in use", GP_ERROR);
            return;
        }
        irc->RequestRemove(scrollback);
    }
}

void Session::processPBResync(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_RESYNC_SCROLLBACK_PB);
        return;
    }

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

    // Now we need to notify all clients about this change as well
    this->SendToOtherSessions(GP_CMD_RESYNC_SCROLLBACK_PB, parameters);
}
