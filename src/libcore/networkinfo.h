//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2021

#ifndef NETWORKINFO_H
#define NETWORKINFO_H

#include <QString>
#include <QHash>
#include "libcore_global.h"
#include "../libirc/libirc/serveraddress.h"
#include "../libirc/libirc/serializableitem.h"
#include "../libirc/libirc/irc_standards.h"

namespace GrumpyIRC
{
    class NetworkInfo : public libirc::SerializableItem
    {
        public:
            static void Clear();
            static int LastID;
            static QHash<int, NetworkInfo*> NetworksInfo;

            NetworkInfo(QString name, QString host, int port, int identity, bool ssl, int id = -1);
            NetworkInfo(const QHash<QString, QVariant> &hash);
            libirc::ServerAddress ToServerAddress();
            QHash<QString, QVariant> ToHash() override;
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            int ID;
            QString NetworkName;
            QString Hostname;
            int Port = IRC_STANDARD_PORT;
            int PreferredIdentity = -1;
            bool AutoReconnect = false;
            bool SSL = false;
    };
}

#endif // NETWORKINFO_H
