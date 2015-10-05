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
#include "ircsession.h"
#include "../libirc/libirc/serveraddress.h"
#include "../libirc/libircclient/network.h"
#include "../libirc/libircclient/user.h"
#include "../libirc/libircclient/channel.h"
#include "scrollback.h"
#include "exception.h"

using namespace GrumpyIRC;

QMutex IRCSession::Sessions_Lock;
QList<IRCSession*> IRCSession::Sessions;

IRCSession *IRCSession::Open(Scrollback *system_window, libirc::ServerAddress &server, QString network)
{
    if (!server.IsValid())
        throw new GrumpyIRC::Exception("Server object is not valid", BOOST_CURRENT_FUNCTION);
    IRCSession *sx = new IRCSession(system_window);
    QString network_name = network;
    if (network.isEmpty())
        network_name = server.GetHost();
    sx->Connect(new libircclient::Network(server, network_name));
    return sx;
}

IRCSession::IRCSession(Scrollback *system)
{
    this->systemWindow = system;
    this->systemWindow->SetSession(this);
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
    delete this->network;
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
		throw new Exception("You can't connect to ircsession that is active, disconnect first", BOOST_CURRENT_FUNCTION);

	this->systemWindow->InsertText("Connecting to " + Network->GetHost());

	if (this->network)
        delete this->network;

	this->network = Network;
    connect(this->network, SIGNAL(Event_ConnectionFailure(QAbstractSocket::SocketError)), this, SLOT(OnConnectionFail(QAbstractSocket::SocketError)));
    connect(this->network, SIGNAL(Event_RawIncoming(QByteArray)), this, SLOT(OnIncomingRawMessage(QByteArray)));
    connect(this->network, SIGNAL(Event_SelfJoin(libircclient::Channel*)), this, SLOT(OnIRCSelfJoin(libircclient::Channel *)));
    this->network->Connect();
}

bool IRCSession::IsConnected()
{
    if (this->network && this->network->IsConnected())
        return true;

    return false;
}

void IRCSession::OnIRCSelfJoin(libircclient::Channel *channel)
{
    if (channel->GetName().isEmpty())
        throw new GrumpyIRC::Exception("Invalid channel name", BOOST_CURRENT_FUNCTION);
    if (this->channels.contains(channel->GetName()))
        throw new GrumpyIRC::Exception("This window name already exists", BOOST_CURRENT_FUNCTION);
    // we just joined a new channel, let's add a scrollback for it
    Scrollback *window = Core::GrumpyCore->NewScrollback(this->systemWindow, channel->GetName());
    this->channels.insert(channel->GetName(), window);
    
}

void IRCSession::OnIncomingRawMessage(QByteArray message)
{
    this->systemWindow->InsertText(QString(message));
}

void IRCSession::OnConnectionFail(QAbstractSocket::SocketError er)
{
    this->systemWindow->InsertText("Connection failed");
}

