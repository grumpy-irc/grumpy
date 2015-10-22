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
    this->SendProtocolCommand("HELLO", parameters);
}

void GrumpydSession::OnIncomingCommand(QString text, QHash<QString, QVariant> parameters)
{
    if (text == "UNKNOWN")
    {
        if (parameters.contains("unrecognized"))
            this->systemWindow->InsertText(QString("Grumpyd didn't recognize this command: ") + parameters["unrecognized"].toString());
    } else if (text == "HELLO")
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
    } else if (text == "LOGIN_FAIL")
    {
        this->closeError("Invalid username or password provided");
    } else if (text == "LOGIN_OK")
    {
        this->systemWindow->InsertText("Synchronizing networks");
        this->SendProtocolCommand("NETWORK_INFO");
    } else if (text == "SCROLLBACK_LOAD_NEW_ITEM")
    {
        this->processNewScrollbackItem(parameters);
    } else if (text == "NETWORK_INFO")
    {
        this->processNetwork(parameters);
    } else if (text == "PERMDENY")
    {
        QString source = "unknown request";
        if (parameters.contains("source"))
            source = parameters["source"].toString();
        this->systemWindow->InsertText("Permission denied: " + source);
    } else
    {
        QHash<QString, QVariant> params;
        params.insert("source", text);
        this->SendProtocolCommand("UNKNOWN", params);
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
    foreach (IRCSession *sx, this->sessionList.values())
    {
        if (sx->GetSID() == sid)
        {
            session = sx;
            break;
        }
    }
    if (!session)
    {
        this->systemWindow->InsertText("Received scrollback item for scrollback for which network couldn't be found, SID: " + QString::number(sid));
        return;
    }
    // Now we need to find the widget
    Scrollback *window = session->GetScrollback(hash["scrollback_name"].toString());
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

void GrumpydSession::closeError(QString error)
{
    this->Disconnect();
    this->systemWindow->SetDead(true);
    this->systemWindow->InsertText("Connection failure: " + error);
}

