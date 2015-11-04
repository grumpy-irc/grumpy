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
#include "../libcore/eventhandler.h"
#include "corewrapper.h"
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
    this->MaxScrollbackSyncItems = 800;
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
    this->SessionState = State_Login;
    connect(this->protocol, SIGNAL(Event_IncomingCommand(QString,QHash<QString,QVariant>)), this, SLOT(OnCommand(QString,QHash<QString,QVariant>)));
    connect(this->protocol, SIGNAL(Event_Disconnected()), this, SLOT(OnDisconnected()));
    this->protocol->ResolveSignals();
    GRUMPY_LOG("New session (" + QString::number(this->SID) + ") from " + this->peer);
    return;

    failure:
        this->SessionState = State_Offline;
        this->protocol = NULL;
        this->socket->close();
}

Session::~Session()
{
    // deletion of socket is performed by destructor of protocol
    GRUMPY_LOG("Session for " + this->peer + " destroyed");
    delete this->protocol;
    if (this->loggedUser)
    {
        // Remove the session from list of sessions this user has open
        this->loggedUser->RemoveSession(this);
    }
    sessions_lock->lock();
    SessionList.removeOne(this);
    sessions_lock->unlock();
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

void Session::SendToEverySession(QString command, QHash<QString, QVariant> parameters)
{
    if (!this->loggedUser)
        return;

    foreach (Session *xx, this->loggedUser->GetGPSessions())
        xx->protocol->SendProtocolCommand(command, parameters);
}

void Session::TransferError(QString source, QString description, int id)
{
    QHash<QString, QVariant> params;
    params.insert("id", QVariant(id));
    params.insert("description", QVariant(description));
    params.insert("source", QVariant(source));
    this->protocol->SendProtocolCommand(GP_CMD_ERROR, params);
}

void Session::PermissionDeny(QString source)
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
    unsigned long long sid = parameters["scrollback_id"].toULongLong();
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
    QString rx = parameters["text"].toString();
    irc->SendMessage(scrollback, rx);
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

void Session::OnCommand(QString text, QHash<QString, QVariant> parameters)
{
    text = text.toUpper();
    if (text == GP_CMD_HELLO)
    {
        if (parameters.contains("version"))
            GRUMPY_DEBUG(QString("SID ") + QString::number(this->SID) + QString(" version ") + parameters["version"].toString(), 1);
        // respond to HELLO which is a first command that is meant to be sent by a client to server and only server
        // can respond to it
        QHash<QString, QVariant> params;
        params.insert("version", QVariant(QString("Grumpyd ") + GRUMPY_VERSION_STRING));
        params.insert("authentication_required", QVariant(true));
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
    this->loggedUser = User::Login(parameters["username"].toString(), parameters["password"].toString());
    if (this->loggedUser)
    {
        if (!this->IsAuthorized(PRIVILEGE_LOGIN))
        {
            this->PermissionDeny(GP_CMD_LOGIN);
            return;
        }
        this->protocol->SendProtocolCommand(GP_CMD_LOGIN_OK);
        GRUMPY_LOG("SID " + QString::number(this->SID) + " identified to " + this->loggedUser->GetName());
        this->loggedUser->InsertSession(this);
    } else
    {
        GRUMPY_LOG("SID " + QString::number(this->SID) + " failed to login to name " + parameters["username"].toString());
        this->protocol->SendProtocolCommand(GP_CMD_LOGIN_FAIL);
    }
}


