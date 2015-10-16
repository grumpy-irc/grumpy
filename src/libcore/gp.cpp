//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "exception.h"
#include "gp.h"

using namespace GrumpyIRC;

GP::GP()
{

}

GP::~GP()
{

}

bool GP::IsConnected() const
{
    if (!this->socket)
        return false;
    return this->socket->isOpen();
}

void GP::OnPing()
{

}

void GP::OnPingSend()
{

}

void GP::OnError(QAbstractSocket::SocketError er)
{

}

void GP::OnReceive()
{

}

void GP::OnConnected()
{

}

static QByteArray ToArray(QHash<QString, QVariant> data)
{
    QByteArray result;
    QDataStream stream(result);
    stream << data;
    return result;
}

static QByteArray ToArray(int number)
{
    QByteArray result;
    QDataStream stream(result);
    stream << qint64(number);
    return result;
}

void GrumpydSession::SendPacket(QHash<QString, QVariant> packet)
{
    // Current format of every packet is extremely simple
    // First GP_HEADER_SIZE bytes are the size of packet
    // Following bytes are the packet itself
    QByteArray result = ToArray(packet);
    QByteArray header = ToArray(result.size());
    if (header.size() != GP_HEADER_SIZE)
        throw new Exception("Invalid header size: " + QString::number(header.size()), BOOST_CURRENT_FUNCTION);
    result.prepend(ToArray(result.size()));
    this->socket->write(result);
    this->socket->flush();
}


