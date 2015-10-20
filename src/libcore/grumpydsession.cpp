//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "grumpydsession.h"
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
    QHash<QString, QVariant> parameters;
    parameters.insert("version", QString(GRUMPY_VERSION_STRING));
    this->SendProtocolCommand("HELLO", parameters);
}

void GrumpydSession::OnIncomingCommand(QString text, QHash<QString, QVariant> parameters)
{

}

