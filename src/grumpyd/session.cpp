//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "session.h"

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
    this->protocol = new GP(socket);
    this->SessionState = State_Login;
    sessions_lock->lock();
    this->SID = lSID++;
    SessionList.append(this);
    sessions_lock->unlock();
    connect(this->protocol, SIGNAL(Event_IncomingCommand(QString,QHash<QString,QVariant>)), this, SLOT(OnCommand(QString,QHash<QString,QVariant>)));
    this->protocol->ResolveSignals();
}

Session::~Session()
{
    // deletion of socket is performed by destructor of protocol
    delete this->protocol;
    sessions_lock->lock();
    SessionList.removeOne(this);
    sessions_lock->unlock();
}

void Session::run()
{

}

unsigned long Session::GetSID()
{
    return this->SID;
}

void Session::OnCommand(QString text, QHash<QString, QVariant> parameters)
{
    if (text == "HELLO")
    {
        // respond to HELLO which is a first command that is meant to be sent by a client to server and only server
        // can respond to it
        this->protocol->SendProtocolCommand("HELLO");
    }
}


