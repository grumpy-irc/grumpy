//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2021

#include "networkinfo.h"

using namespace GrumpyIRC;

QHash<int, NetworkInfo*> NetworkInfo::NetworksInfo;
int NetworkInfo::LastID = 0;

void NetworkInfo::Clear()
{
    QList<NetworkInfo*> list = NetworkInfo::NetworksInfo.values();
    foreach (NetworkInfo *network_info, list)
    {
        delete network_info;
    }
    NetworkInfo::NetworksInfo.clear();
    NetworkInfo::LastID = 0;
}

NetworkInfo::NetworkInfo(QString name, QString host, int port, int identity, int id)
{
    if (id < 0)
    {
        this->ID = NetworkInfo::LastID++;
    } else
    {
        this->ID = id;
        if (id > NetworkInfo::LastID)
            NetworkInfo::LastID = id + 1;
    }

    this->NetworkName = name;
    this->Hostname = host;
    this->Port = port;
    this->PreferredIdentity = identity;
}

NetworkInfo::NetworkInfo(const QHash<QString, QVariant> &hash)
{
    this->LoadHash(hash);
    if (this->ID > LastID)
        LastID = this->ID + 1;
}

QHash<QString, QVariant> NetworkInfo::ToHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(ID);
    SERIALIZE(NetworkName);
    SERIALIZE(Hostname);
    SERIALIZE(PreferredIdentity);
    SERIALIZE(SSL);
    SERIALIZE(AutoReconnect);
    SERIALIZE(Port);
    return hash;
}

void NetworkInfo::LoadHash(const QHash<QString, QVariant> &hash)
{
    UNSERIALIZE_STRING(NetworkName);
    UNSERIALIZE_INT(ID);
    UNSERIALIZE_STRING(Hostname);
    UNSERIALIZE_INT(PreferredIdentity);
    UNSERIALIZE_BOOL(SSL);
    UNSERIALIZE_INT(Port);
    UNSERIALIZE_BOOL(AutoReconnect);
}
