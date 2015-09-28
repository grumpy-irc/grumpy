//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "ircsession.h"
#include "scrollback.h"
#include "exception.h"

using namespace GrumpyIRC;

QMutex IRCSession::Sessions_Lock;
QList<IRCSession*> IRCSession::Sessions;

IRCSession *IRCSession::Open(Scrollback *system_window, QString hostname, QString network, QString nick, QString password, bool ssl)
{
    IRCSession *sx = new IRCSession(system_window);
    QString network_name = network;
    if (network.isEmpty())
        network_name = hostname;
    sx->Connect(new libircclient::Network(network_name, hostname, nick, ssl, password));
    return sx;
}

IRCSession::IRCSession(Scrollback *system)
{
    this->systemWindow = system;
	this->network = NULL;
    IRCSession::Sessions_Lock.lock();
    IRCSession::Sessions.append(this);
    IRCSession::Sessions_Lock.unlock();
}

IRCSession::~IRCSession()
{
    IRCSession::Sessions_Lock.lock();
    IRCSession::Sessions.removeOne(this);
    IRCSession::Sessions_Lock.unlock();
}

Scrollback *IRCSession::GetSystemWindow()
{
    return this->systemWindow;
}

libircclient::Network *IRCSession::GetNetwork()
{
    return this->network;
}

void IRCSession::Connect(libircclient::Network *Network)
{
    if (this->IsConnected())
		throw new Exception();

	this->systemWindow->InsertText("Connecting to " + Network->GetHost());
	if (this->network)
	{
		throw new GrumpyIRC::Exception("You can't connect to ircsession that is active, disconnect first");
	}
	this->network = Network;
    this->network->Connect();
}

bool IRCSession::IsConnected()
{
    if (this->network && this->network->IsConnected())
        return true;

    return false;
}

