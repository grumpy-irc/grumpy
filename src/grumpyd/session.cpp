//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include <QHostAddress>
#include <QHash>
#include <QSslSocket>
#include <QDir>
#include "../libirc/libirc/serveraddress.h"
#include "../libirc/libircclient/network.h"
#include "../libcore/core.h"
#include "../libcore/grumpydsession.h"
#include "../libcore/eventhandler.h"
#include "../libcore/generic.h"
#include "script_engine/grumpydscript.h"
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
    this->IsAway = false;
    this->usingSsl = ssl;
    this->protocol = nullptr;
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
    this->loggedUser = nullptr;
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
    connect(this->protocol, SIGNAL(Event_ConnectionFailed(QString,int)), this, SLOT(OnGPError(QString,int)));
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
    GRUMPY_LOG("Session " + QString::number(this->SID) + " for " + this->peer + " destroyed");
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

bool Session::IsAuthorized(const QString &permission)
{
    if (!this->loggedUser)
        return false;

    return this->loggedUser->IsAuthorized(permission);
}

void Session::SendToEverySession(gp_command_t command, const QHash<QString, QVariant>& parameters)
{
    if (!this->loggedUser)
        return;

    foreach (Session *xx, this->loggedUser->GetGPSessions())
        xx->protocol->SendProtocolCommand(command, parameters);
}

void Session::SendToOtherSessions(gp_command_t command, const QHash<QString, QVariant>& parameters)
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
    Scrollback *result = nullptr;
    if (!this->loggedUser)
        return nullptr;
    foreach (SyncableIRCSession *session, this->loggedUser->GetSIRCSessions())
    {
        result = session->GetScrollback(scrollback_id);
        if (result)
            return result;
    }
    return nullptr;
}

void Session::TransferError(gp_command_t source, const QString& description, int id)
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

void Session::Kick()
{
    this->TransferError(0, "You were kicked from grumpyd", 0);
    this->Disconnect();
}

void Session::Shutdown()
{
    this->TransferError(0, "Grumpyd is shutting down", 0);
    this->Disconnect();
}

void Session::Disconnect()
{
    this->SessionState = State_Killing;
    this->IsRunning = false;
    this->protocol->Disconnect();
    this->socket = nullptr;
}

void Session::OnDisconnected()
{
    this->SessionState = State_Exiting;
    this->IsRunning = false;
    this->socket = nullptr;
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
    irc->RequestDisconnect(nullptr, parameters["reason"].toString(), false);
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
        this->TransferError(GP_CMD_MESSAGE, "Scrollback not found", GP_ESCROLLBACKNOTFOUND);
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
        irc->RetrieveChannelBanList(nullptr, parameters["channel_name"].toString());
    else if (tx == "+I")
        irc->RetrieveChannelInviteList(nullptr, parameters["channel_name"].toString());
    else if (tx == "+e")
        irc->RetrieveChannelExceptionList(nullptr, parameters["channel_name"].toString());
}

void Session::processQuery(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_QUERY);
        return;
    }

    // We have the original ID of scrollback so let's find it here
    unsigned int nsid = parameters["network_id"].toUInt();
    scrollback_id_t sid = parameters["scrollback_id"].toUInt();
    if (!parameters.contains("target"))
    {
        this->TransferError(GP_CMD_QUERY, "No target", GP_ENOUSER);
        return;
    }
    QString target = parameters["target"].toString();
    // let's find the network
    SyncableIRCSession* irc = this->loggedUser->GetSIRCSession(nsid);
    if (!irc)
    {
        GRUMPY_ERROR("Unable to find network with id " + QString::number(nsid) + " message can't be delivered for user " + this->loggedUser->GetName());
        this->TransferError(GP_CMD_QUERY, "Network not found", GP_ENETWORKNOTFOUND);
        return;
    }
    Scrollback *scrollback = irc->GetScrollback(sid);
    if (!scrollback)
    {
        GRUMPY_ERROR("Unable to find scrollback id " + QString::number(sid) + " message can't be delivered for user " + this->loggedUser->GetName());
        this->TransferError(GP_CMD_QUERY, "Scrollback not found", GP_ESCROLLBACKNOTFOUND);
        return;
    }
    QString msg;
    if (parameters.contains("message"))
        msg = parameters["message"].toString();
    irc->Query(scrollback, target, msg);
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
        if (user->GetRole() != nullptr)
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

void Session::processLockUser(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_LOCK_USER))
    {
        this->PermissionDeny(GP_CMD_SYS_LOCK_USER);
        return;
    }

    if (!parameters.contains("id"))
    {
        this->TransferError(GP_CMD_SYS_LOCK_USER, "No user provided", GP_ENOUSER);
        return;
    }

    User *target = User::GetUser(parameters["id"].toUInt());
    if (!target)
    {
        this->TransferError(GP_CMD_SYS_LOCK_USER, "User was not found", GP_EWRONGUSER);
        return;
    }

    if (target == this->loggedUser)
    {
        this->TransferError(GP_CMD_SYS_LOCK_USER, "Can't modify self", GP_ESELFTARGET);
        return;
    }

    if (target->IsLocked())
    {
        this->TransferError(GP_CMD_SYS_LOCK_USER, "Already locked", GP_ENOCHANGE);
        return;
    }

    target->Lock();

    if (!parameters.contains("username"))
        parameters.insert("username", target->GetName());

    // Confirm success
    this->protocol->SendProtocolCommand(GP_CMD_SYS_LOCK_USER, parameters);
}

void Session::processUnlockUser(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_UNLOCK_USER))
    {
        this->PermissionDeny(GP_CMD_SYS_UNLOCK_USER);
        return;
    }

    if (!parameters.contains("id"))
    {
        this->TransferError(GP_CMD_SYS_UNLOCK_USER, "No user provided", GP_ENOUSER);
        return;
    }

    User *target = User::GetUser(parameters["id"].toUInt());
    if (!target)
    {
        this->TransferError(GP_CMD_SYS_UNLOCK_USER, "User was not found", GP_EWRONGUSER);
        return;
    }

    if (target == this->loggedUser)
    {
        this->TransferError(GP_CMD_SYS_UNLOCK_USER, "Can't modify self", GP_ESELFTARGET);
        return;
    }

    if (!target->IsLocked())
    {
        this->TransferError(GP_CMD_SYS_LOCK_USER, "Already unlocked", GP_ENOCHANGE);
        return;
    }

    target->Unlock();

    if (!parameters.contains("username"))
        parameters.insert("username", target->GetName());

    // Confirm success
    this->protocol->SendProtocolCommand(GP_CMD_SYS_UNLOCK_USER, parameters);
}

void Session::processCreateUser(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_CREATE_USER))
    {
        this->PermissionDeny(GP_CMD_SYS_CREATE_USER);
        return;
    }

    if (!parameters.contains("username") || !parameters.contains("password"))
    {
        this->TransferError(GP_CMD_SYS_CREATE_USER, "Missing user info", GP_EWRONGUSER);
        return;
    }

    QString user_name = parameters["username"].toString().toLower();
    foreach (User *user, User::UserInfo)
    {
        if (user->GetName().toLower() == user_name)
        {
            this->TransferError(GP_CMD_SYS_CREATE_USER, "User already exists", GP_EWRONGUSER);
            return;
        }
    }

    if (!User::IsValid(user_name))
    {
        this->TransferError(GP_CMD_SYS_CREATE_USER, "Invalid user name", GP_EWRONGUSER);
        return;
    }

    Role *role = Role::DefaultRole;
    if (parameters.contains("role"))
    {
        if (!this->IsAuthorized(PRIVILEGE_GRANT_ANY_ROLE))
        {
            this->PermissionDeny(GP_CMD_SYS_CREATE_USER);
            return;
        }
        if (!Role::Roles.contains(parameters["role"].toString()))
        {
            this->TransferError(GP_CMD_SYS_CREATE_USER, "No such role", 0);
            return;
        }

        role = Role::Roles[parameters["role"].toString()];
    }

    // Register a new user account
    User *new_user = User::CreateUser(parameters["username"].toString(), User::EncryptPw(parameters["password"].toString()), role);

    QHash<QString, QVariant> result;
    result.insert("done", QVariant(true));
    result.insert("username", new_user->GetName());
    result.insert("id", new_user->GetID());
    this->protocol->SendProtocolCommand(GP_CMD_SYS_CREATE_USER, result);
}

void Session::processRemoveUser(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_REMOVE_USER))
    {
        this->PermissionDeny(GP_CMD_SYS_REMOVE_USER);
        return;
    }

    if (!parameters.contains("id"))
    {
        this->TransferError(GP_CMD_SYS_REMOVE_USER, "No user provided", GP_ENOUSER);
        return;
    }

    User *target = User::GetUser(parameters["id"].toUInt());
    if (!target)
    {
        this->TransferError(GP_CMD_SYS_REMOVE_USER, "User was not found", GP_EWRONGUSER);
        return;
    }

    if (target == this->loggedUser)
    {
        this->TransferError(GP_CMD_SYS_REMOVE_USER, "Can't remove self", GP_ESELFTARGET);
        return;
    }
    QString username = target->GetName();
    // Kick & lock user to ensure no active sessions for this user exist
    target->Lock();

    if (User::RemoveUser(target->GetID()))
    {
        if (!parameters.contains("username"))
            parameters.insert("username", username);
        this->protocol->SendProtocolCommand(GP_CMD_SYS_REMOVE_USER, parameters);
    } else
    {
        this->TransferError(GP_CMD_SYS_REMOVE_USER, "Unable to remove user", 0);
    }
}

void Session::processStorageSet(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_STORAGE))
    {
        this->PermissionDeny(GP_CMD_STORAGE_SET);
        return;
    }

    if (!parameters.contains("key") || !parameters.contains("data"))
    {
        this->TransferError(GP_CMD_STORAGE_SET, "Invalid data", GP_ERROR);
        return;
    }

    bool ok = this->loggedUser->StorageSet(parameters["key"].toString(), parameters["data"].toByteArray());

    if (ok)
    {
        QHash<QString, QVariant> result;
        result.insert("key", parameters["key"].toString());
        this->protocol->SendProtocolCommand(GP_CMD_STORAGE_SET, result);
    } else
    {
        this->TransferError(GP_CMD_STORAGE_SET, "Not enough space in personal storage", GP_ENOSPACE);
        return;
    }
}

void Session::processStorageDel(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_STORAGE))
    {
        this->PermissionDeny(GP_CMD_STORAGE_DEL);
        return;
    }

    if (!parameters.contains("key"))
    {
        this->TransferError(GP_CMD_STORAGE_DEL, "Missing key", GP_ERROR);
        return;
    }

    QString key = parameters["key"].toString();
    if (!this->loggedUser->StorageContains(key))
    {
        this->TransferError(GP_CMD_STORAGE_GET, "No such key", GP_ERROR);
    } else
    {
        this->loggedUser->StorageDelete(key);
        QHash<QString, QVariant> result;
        result.insert("key", parameters["key"].toString());
        this->protocol->SendProtocolCommand(GP_CMD_STORAGE_DEL, result);
    }
}

void Session::processStorageGet(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_STORAGE))
    {
        this->PermissionDeny(GP_CMD_STORAGE_GET);
        return;
    }

    if (!parameters.contains("key"))
    {
        this->TransferError(GP_CMD_STORAGE_GET, "Missing key", GP_ERROR);
        return;
    }

    QString key = parameters["key"].toString();
    if (!this->loggedUser->StorageContains(key))
    {
        this->TransferError(GP_CMD_STORAGE_GET, "No such key", GP_ERROR);
    } else
    {
        QHash<QString, QVariant> result;
        result.insert("key", parameters["key"].toString());
        result.insert("data", this->loggedUser->StorageGet(key));
        this->protocol->SendProtocolCommand(GP_CMD_STORAGE_GET, result);
    }
}

void Session::processAway(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_AWAY);
        return;
    }

    if (!parameters.contains("away"))
    {
        this->TransferError(GP_CMD_AWAY, "Missing state", GP_ERROR);
        return;
    }

    this->AwayReason.clear();
    this->IsAway = parameters["away"].toBool();
    if (parameters.contains("reason"))
    {
        this->AwayReason = parameters["reason"].toString();
    }

    // Let session handler know about this
    emit this->OnAway();
    this->loggedUser->LastAwayReason = this->AwayReason;
    this->loggedUser->UpdateAway();
}

void Session::processSniffer(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_SNIFFER))
    {
        this->PermissionDeny(GP_CMD_GET_SNIFFER);
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
    QList<QVariant> result;
    QList<NetworkSniffer_Item> items = irc->GetSniffer();
    foreach (NetworkSniffer_Item item, items)
        result.append(item.ToHash());
    parameters.insert("sniffer", result);
    this->protocol->SendProtocolCommand(GP_CMD_GET_SNIFFER, parameters);
}

void Session::processInstallScript(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_MANAGE_SCRIPT))
    {
        this->PermissionDeny(GP_CMD_SYS_INSTALL_SCRIPT);
        return;
    }

    if (!parameters.contains("path"))
    {
        this->TransferError(GP_CMD_SYS_INSTALL_SCRIPT, "Missing path", GP_ERROR);
        return;
    }
    if (!parameters.contains("source"))
    {
        this->TransferError(GP_CMD_SYS_INSTALL_SCRIPT, "Missing source", GP_ERROR);
        return;
    }

    QString extension_path = parameters["path"].toString();
    QString extension_source = parameters["source"].toString();

    if (!Generic::IsValidFileName(extension_path) || !extension_path.toLower().endsWith(".js"))
    {
        this->TransferError(GP_CMD_SYS_INSTALL_SCRIPT, "Script path is not a valid file name", GP_ERROR);
        return;
    }

    // Try to store this as a file
    QString path = CONF->GetScriptPath() + QDir::separator() + extension_path;
    if (QFile::exists(path))
    {
        this->TransferError(GP_CMD_SYS_INSTALL_SCRIPT, "Script with this name is already located on disk", GP_ERROR);
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        GRUMPY_ERROR("Unable to open " + path + " for writing");
        this->TransferError(GP_CMD_SYS_INSTALL_SCRIPT, "Unable to open " + path + " for writing", GP_ERROR);
        return;
    }

    if (!file.write(extension_source.toLatin1()))
    {
        GRUMPY_ERROR("Unable to write extension source into " + path);
        this->TransferError(GP_CMD_SYS_INSTALL_SCRIPT, "Unable to write data into " + path + " (storage full?)", GP_ERROR);
        return;
    }

    file.close();

    GRUMPY_LOG("User " + this->loggedUser->GetName() +  " is installing script " + extension_path);

    QString error;
    GrumpydScript *ex = new GrumpydScript();
    if (!ex->LoadSrc(path, extension_source, &error))
    {
        GRUMPY_ERROR("Unable to load script " + extension_path + ": " + error);
        this->TransferError(GP_CMD_SYS_INSTALL_SCRIPT, error, GP_ERROR);
        delete ex;
        if (!QFile::remove(path))
        {
            GRUMPY_ERROR("Unable to delete: " + path);
        }
        return;
    }

    // No need to send this back
    parameters.remove("source");
    this->protocol->SendProtocolCommand(GP_CMD_SYS_INSTALL_SCRIPT, parameters);
}

void Session::processRemoveScript(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_MANAGE_SCRIPT))
    {
        this->PermissionDeny(GP_CMD_SYS_UNINST_SCRIPT);
        return;
    }

    ScriptExtension *e = nullptr;

    if (parameters.contains("path"))
        e = ScriptExtension::GetExtensionByPath(parameters["path"].toString());
    else if (parameters.contains("name"))
        e = ScriptExtension::GetExtensionByName(parameters["name"].toString());

    if (e == nullptr)
    {
        GRUMPY_DEBUG("Request to delete nonexistent script", 1);
        this->TransferError(GP_CMD_SYS_UNINST_SCRIPT, "Extension not found", GP_ERROR);
        return;
    }

    // Remove from disk
    QString extension_path = e->GetPath();
    if (!extension_path.startsWith(CONF->GetScriptPath()))
        extension_path = CONF->GetScriptPath() + QDir::separator() + extension_path;
    if (!QFile::exists(extension_path))
    {
        GRUMPY_ERROR("Unable to find: " + extension_path + " not deleting extension from storage");
    } else
    {
        if (!QFile::remove(extension_path))
        {
            GRUMPY_DEBUG("Unable to rm: " + extension_path, 1);
            this->TransferError(GP_CMD_SYS_UNINST_SCRIPT, "Unable to delete " + extension_path, GP_ERROR);
            return;
        }
    }

    QString name = e->GetName();

    e->Unload();
    delete e;

    GRUMPY_DEBUG("Successfuly removed extension " + name, 1);

    this->protocol->SendProtocolCommand(GP_CMD_SYS_UNINST_SCRIPT, parameters);
}

void Session::processScriptLS(QHash<QString, QVariant> parameters)
{
    Q_UNUSED(parameters);
    if (!this->IsAuthorized(PRIVILEGE_READ_SCRIPT))
    {
        this->PermissionDeny(GP_CMD_SYS_LIST_SCRIPT);
        return;
    }

    QList<QVariant> script_list;
    foreach (ScriptExtension *s, ScriptExtension::GetExtensions())
    {
        QHash<QString, QVariant> data;
        data.insert("name", s->GetName());
        QString fixedpath = s->GetPath();
        if (fixedpath.contains(QDir::separator()))
            fixedpath = fixedpath.mid(fixedpath.lastIndexOf(QDir::separator()) + 1);
        data.insert("path", fixedpath);
        data.insert("is_working", s->IsWorking());
        data.insert("author", s->GetAuthor());
        data.insert("version", s->GetVersion());
        data.insert("description", s->GetDescription());
        script_list.append(QVariant(data));
    }

    QHash<QString, QVariant> response;
    response.insert("list", QVariant(script_list));

    this->protocol->SendProtocolCommand(GP_CMD_SYS_LIST_SCRIPT, response);
}

void Session::processScriptReadSource(QHash<QString, QVariant> parameters)
{
    Q_UNUSED(parameters);
    if (!this->IsAuthorized(PRIVILEGE_READ_SCRIPT))
    {
        this->PermissionDeny(GP_CMD_SYS_READ_SCRIPT_SOURCE_CODE);
        return;
    }

    ScriptExtension *e = nullptr;

    if (parameters.contains("path"))
        e = ScriptExtension::GetExtensionByPath(parameters["path"].toString());
    else if (parameters.contains("name"))
        e = ScriptExtension::GetExtensionByName(parameters["name"].toString());

    if (e == nullptr)
    {
        GRUMPY_DEBUG("Request to get a source code of nonexistent script", 1);
        this->TransferError(GP_CMD_SYS_READ_SCRIPT_SOURCE_CODE, "Script not found", GP_ERROR);
        return;
    }

    QHash<QString, QVariant> data;
    data.insert("name", e->GetName());
    data.insert("id", e->GetPath());
    data.insert("source", e->GetSource());
    data.insert("is_working", e->IsWorking());
    data.insert("author", e->GetAuthor());
    data.insert("version", e->GetVersion());
    data.insert("description", e->GetDescription());
    this->protocol->SendProtocolCommand(GP_CMD_SYS_READ_SCRIPT_SOURCE_CODE, data);
}

void Session::OnCommand(gp_command_t text, QHash<QString, QVariant> parameters)
{
    switch (text)
    {
        case GP_CMD_HELLO:
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
            // For now we show uptime to everyone who connects, maybe limit this to admins only in future
            QDateTime uptime = CONF->GetConfiguration()->GetStartupDateTime();
            int days, hours, minutes, seconds;
            Generic::SecondsToTimeSpan(uptime.secsTo(QDateTime::currentDateTime()), &days, &hours, &minutes, &seconds);
            QString uptime_str = "Since " + uptime.toString() + ": " + QString::number(days) + " days " + Generic::DoubleDigit(hours) + ":" +
                                 Generic::DoubleDigit(minutes) + ":" + Generic::DoubleDigit(seconds);
            params.insert("uptime", uptime_str);
            this->protocol->SendProtocolCommand(GP_CMD_HELLO, params);
        }
            break;
        case GP_CMD_UNKNOWN:
            if (!parameters.contains("unrecognized"))
                return;
            GRUMPY_DEBUG(QString("SID ") + QString::number(this->SID) + QString(" doesn't understand protocol ") + parameters["unrecognized"].toString(), 2);
            break;
        case GP_CMD_RAW:
            this->processCommand(parameters);
            break;
        case GP_CMD_LOGIN:
            this->processLogin(parameters);
            break;
        case GP_CMD_IRC_QUIT:
            this->processIrcQuit(parameters);
            break;
        case GP_CMD_MESSAGE:
            this->processMessage(parameters);
            break;
        case GP_CMD_NETWORK_INFO:
            this->processNetworks();
            break;
        case GP_CMD_SERVER:
            this->processNew(parameters);
            break;
        case GP_CMD_RECONNECT:
            this->processReconnect(parameters);
            break;
        case GP_CMD_SCROLLBACK_PARTIAL_RESYNC:
            this->processResync(parameters);
            break;
        case GP_CMD_REQUEST_ITEMS:
            this->processRequest(parameters);
            break;
        case GP_CMD_REQUEST_INFO:
            this->processInfo(parameters);
            break;
        case GP_CMD_REMOVE:
            this->processRemove(parameters);
            break;
        case GP_CMD_INIT:
            this->processSetup(parameters);
            break;
        case GP_CMD_OPTIONS:
            this->processOptions(parameters);
            break;
        case GP_CMD_SYS_LIST_USER:
            this->processUserList(parameters);
            break;
        case GP_CMD_RESYNC_SCROLLBACK_PB:
            this->processPBResync(parameters);
            break;
        case GP_CMD_SYS_LOCK_USER:
            this->processLockUser(parameters);
            break;
        case GP_CMD_SYS_UNLOCK_USER:
            this->processUnlockUser(parameters);
            break;
        case GP_CMD_SYS_CREATE_USER:
            this->processCreateUser(parameters);
            break;
        case GP_CMD_SYS_REMOVE_USER:
            this->processRemoveUser(parameters);
            break;
        case GP_CMD_STORAGE_DEL:
            this->processStorageDel(parameters);
            break;
        case GP_CMD_STORAGE_GET:
            this->processStorageGet(parameters);
            break;
        case GP_CMD_STORAGE_SET:
            this->processStorageSet(parameters);
            break;
        case GP_CMD_QUERY:
            this->processQuery(parameters);
            break;
        case GP_CMD_AWAY:
            this->processAway(parameters);
            break;
        case GP_CMD_SYS_READ_SCRIPT_SOURCE_CODE:
            this->processScriptReadSource(parameters);
            break;
        case GP_CMD_SYS_UNINST_SCRIPT:
            this->processRemoveScript(parameters);
            break;
        case GP_CMD_SYS_INSTALL_SCRIPT:
            this->processInstallScript(parameters);
            break;
        case GP_CMD_SYS_LIST_SCRIPT:
            this->processScriptLS(parameters);
            break;
        case GP_CMD_GET_SNIFFER:
            this->processSniffer(parameters);
            break;
        case GP_CMD_HIDE_SB:
            this->processHideSB(parameters);
            break;
        default:
        {
            // We received some unknown packet, send it back to client so that it at least knows we don't support this
            QHash<QString, QVariant> params;
            params.insert("unrecognized", QVariant(text));
            this->protocol->SendProtocolCommand(GP_CMD_UNKNOWN, params);
        }
            break;
    }
}

void Session::OnGPError(QString text, int code)
{
    GRUMPY_LOG("SID " + QString::number(this->SID) + " connection terminated. Failure code " + QString::number(code) + ": " + text);
    this->SessionState = State_Offline;
    this->IsRunning = false;
    this->socket = nullptr;
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
        if (this->loggedUser->IsLocked())
        {
            GRUMPY_LOG("SID " + QString::number(this->SID) + " failed to login (is locked) to name " + parameters["username"].toString());
            this->TransferError(GP_CMD_LOGIN, "Account is locked", GP_ELOCKED);
            this->protocol->SendProtocolCommand(GP_CMD_LOGIN_FAIL);
            // Remove the pointer - we don't consider this a valid login
            this->loggedUser = nullptr;
            return;
        }
        if (!this->IsAuthorized(PRIVILEGE_LOGIN))
        {
            this->PermissionDeny(GP_CMD_LOGIN);
            // Remove the pointer - we don't consider this a valid login
            this->loggedUser = nullptr;
            return;
        }
        QHash<QString, QVariant> param;
        param.insert("logged", this->loggedUser->GetSessionCount());
        this->protocol->SendProtocolCommand(GP_CMD_LOGIN_OK, param);
        GRUMPY_LOG("SID " + QString::number(this->SID) + " identified as " + this->loggedUser->GetName());
        this->loggedUser->InsertSession(this);
    } else
    {
        GRUMPY_LOG("SID " + QString::number(this->SID) + " failed to login (wrong user / pass) as name " + parameters["username"].toString());
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
    Grumpyd::GetBackend()->UpdateScrollback(this->loggedUser, origin);
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

void Session::processHideSB(QHash<QString, QVariant> parameters)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny(GP_CMD_HIDE_SB);
        return;
    }

    if (!parameters.contains("scrollback_id"))
        return;
    Scrollback *scrollback = this->GetScrollback(parameters["scrollback_id"].toUInt());
    if (!scrollback)
        return;
    if (!parameters.contains("hide"))
        return;

    if (!scrollback->IsHideable())
    {
        this->TransferError(GP_CMD_HIDE_SB, "Scrollback is not hideable", GP_ERROR);
        return;
    }

    bool hide = parameters["hide"].toBool();
    if (hide)
        scrollback->Hide();
    else
        scrollback->Show();

    // Store this in DB
    Grumpyd::GetBackend()->UpdateScrollback(this->loggedUser, scrollback);

    this->SendToEverySession(GP_CMD_HIDE_SB, parameters);
}
