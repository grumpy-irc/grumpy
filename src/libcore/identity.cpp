//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2021

#include "identity.h"

using namespace GrumpyIRC;

int Identity::LastID = 0;
QHash<int, Identity*> Identity::Identities;

void Identity::Clear()
{
    QList<Identity*> list = Identity::Identities.values();
    foreach (Identity *identity, list)
    {
        delete identity;
    }
    Identity::Identities.clear();
    Identity::LastID = 0;
}

Identity::Identity(QHash<QString, QVariant> hash)
{
    this->LoadHash(hash);
    if (this->ID > Identity::LastID)
        Identity::LastID = this->ID + 1;
}

Identity::Identity(QString nick, QString ident, QString real_name, QString away_msg, int id)
{
    if (id < 0)
    {
        this->ID = Identity::LastID++;
    } else
    {
        this->ID = id;
        if (id > Identity::LastID)
            Identity::LastID = id + 1;
    }

    this->Nick = nick;
    this->Ident = ident;
    this->RealName = real_name;
    this->AwayMessage = away_msg;
}

QHash<QString, QVariant> Identity::ToHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(Nick);
    SERIALIZE(Ident);
    SERIALIZE(RealName);
    SERIALIZE(AwayMessage);
    return hash;
}

void Identity::LoadHash(const QHash<QString, QVariant> &hash)
{
    UNSERIALIZE_STRING(Nick);
    UNSERIALIZE_STRING(Ident);
    UNSERIALIZE_STRING(RealName);
    UNSERIALIZE_STRING(AwayMessage);
}

void Identity::ToString()
{
    QString result = this->Nick;
    if (!this->Ident.isEmpty())
        result += "@" + this->Ident;
}
