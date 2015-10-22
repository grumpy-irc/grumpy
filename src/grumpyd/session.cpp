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
#include "corewrapper.h"
#include "user.h"
#include "security.h"
#include "session.h"
#include "../libirc/libirc/serveraddress.h"
#include "../libirc/libircclient/network.h"
#include "syncableircsession.h"
#include "../libcore/core.h"
#include "../libcore/eventhandler.h"

using namespace GrumpyIRC;

QMutex *Session::sessions_lock = new QMutex();
QList<Session*> Session::SessionList;
unsigned long Session::lSID = 0;

QList<Session *> Session::Sessions()
{
    return SessionList;
}

Session::Session(qintptr SocketPtr)
{
    this->socket = new QTcpSocket();
    this->socket->setSocketDescriptor(SocketPtr);
    this->protocol = new libgp::GP(socket);
    this->SessionState = State_Login;
    sessions_lock->lock();
    this->SID = lSID++;
    this->IsRunning = true;
    SessionList.append(this);
    sessions_lock->unlock();
    connect(this->protocol, SIGNAL(Event_IncomingCommand(QString,QHash<QString,QVariant>)), this, SLOT(OnCommand(QString,QHash<QString,QVariant>)));
    this->protocol->ResolveSignals();
    this->loggedUser = NULL;
    GRUMPY_LOG("New session (" + QString::number(this->SID) + ") from " + this->socket->peerAddress().toString());
}

Session::~Session()
{
    // deletion of socket is performed by destructor of protocol
    GRUMPY_LOG("Session for " + this->socket->peerAddress().toString() + " destroyed");
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
    while(this->IsRunning)
    {
        sleep(20000);
    }
}

unsigned long Session::GetSID()
{
    return this->SID;
}

bool Session::IsAuthorized(QString permission)
{
    if (!this->loggedUser)
        return false;

    return true;
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
    this->protocol->SendProtocolCommand("ERROR", params);
}

void Session::PermissionDeny(QString source)
{
    QHash<QString, QVariant> params;
    params.insert("source", QVariant(source));
    this->protocol->SendProtocolCommand("PERMDENY", params);
}

void Session::processNetworks()
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny("NETWORK_INFO");
        return;
    }

    // Send list of serialized irc sessions
    QList<QVariant> sessions;
    QList<SyncableIRCSession*> network_info = this->loggedUser->GetSIRCSessions();
    foreach (SyncableIRCSession *session, network_info)
        sessions.append(QVariant(session->ToHash()));
    QHash<QString, QVariant> params;
    params.insert("sessions", sessions);
    this->protocol->SendProtocolCommand("NETWORK_INFO", params);
}

void Session::processNew(QHash<QString, QVariant> info)
{
    if (!this->IsAuthorized(PRIVILEGE_USE_IRC))
    {
        this->PermissionDeny("SERVER");
        return;
    }

    // User wants to connect to some server
    if (!info.contains("server"))
    {
        this->TransferError("SERVER", "No server host provided", GP_ENOSERVER);
        return;
    }

    libirc::ServerAddress server(info["server"].toHash());
    SyncableIRCSession *session = this->loggedUser->ConnectToIRCServer(server);
    // now we need to deliver message to every session that new connection to a server was open
    QList<QVariant> network_info;
    QHash<QString, QVariant> parameters;
    network_info.append(QVariant(session->ToHash()));
    parameters.insert("sessions", network_info);
    this->SendToEverySession("NETWORK_INFO", parameters);
}

void Session::OnCommand(QString text, QHash<QString, QVariant> parameters)
{
    text = text.toUpper();
    if (text == "HELLO")
    {
        if (parameters.contains("version"))
            GRUMPY_DEBUG(QString("SID ") + QString::number(this->SID) + QString(" version ") + parameters["version"].toString(), 1);
        // respond to HELLO which is a first command that is meant to be sent by a client to server and only server
        // can respond to it
        QHash<QString, QVariant> params;
        params.insert("version", QVariant(QString("Grumpyd ") + GRUMPY_VERSION_STRING));
        params.insert("authentication_required", QVariant(true));
        this->protocol->SendProtocolCommand("HELLO", params);
    } else if (text == "UNKNOWN")
    {
        if (!parameters.contains("unrecognized"))
            return;
        GRUMPY_DEBUG(QString("SID ") + QString::number(this->SID) + QString(" doesn't understand protocol ") + parameters["unrecognized"].toString(), 2);
    } else if (text == "LOGIN")
    {
        // User wants to login
        if (this->loggedUser)
        {
            this->TransferError("LOGIN", "You are already logged in", GP_EALREADYLOGGEDIN);
            return;
        }
        if (!parameters.contains("username") || !parameters.contains("password"))
        {
            this->TransferError("LOGIN", "Invalid parameters", GP_EINVALIDLOGINPARAMS);
            return;
        }
        this->loggedUser = User::Login(parameters["username"].toString(), parameters["password"].toString());
        if (this->loggedUser)
        {
            if (!this->IsAuthorized(PRIVILEGE_LOGIN))
            {
                this->PermissionDeny("LOGIN");
                return;
            }
            this->protocol->SendProtocolCommand("LOGIN_OK");
            this->loggedUser->InsertSession(this);
        } else
        {
            this->protocol->SendProtocolCommand("LOGIN_FAIL");
        }
    } else if (text == "NETWORK_INFO")
    {
        this->processNetworks();
    } else if (text == "SERVER")
    {
        this->processNew(parameters);
    } else
    {
        // We received some unknown packet, send it back to client so that it at least knows we don't support this
        QHash<QString, QVariant> params;
        params.insert("unrecognized", QVariant(text));
        this->protocol->SendProtocolCommand("UNKNOWN", params);
    }
}


