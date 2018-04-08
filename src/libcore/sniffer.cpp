//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "sniffer.h"

using namespace GrumpyIRC;

NetworkSniffer_Item::NetworkSniffer_Item(QByteArray data, bool is_outgoing)
{
    this->_outgoing = is_outgoing;
    this->Text = QString(data);
    this->Time = QDateTime::currentDateTime();
}

QHash<QString, QVariant> NetworkSniffer_Item::ToHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(_outgoing);
    SERIALIZE(Text);
    SERIALIZE(Time);
    return hash;
}

void NetworkSniffer_Item::LoadHash(QHash<QString, QVariant> hash)
{
    UNSERIALIZE_STRING(Text);
    UNSERIALIZE_BOOL(_outgoing);
    UNSERIALIZE_DATETIME(Time);
}
