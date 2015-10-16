//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef GP_H
#define GP_H

#include "libcore_global.h"
#include <QObject>
#include <QHash>
#include <QAbstractSocket>
#include <QString>

#define GP_MAGIC 0x010000
#define GP_HEADER_SIZE 8
#define GP_DEFAULT_PORT 6200

namespace libirc
{
    class ServerAddress;
}

namespace libircclient
{
    class Channel;
    class Network;
    class Parser;
    class User;
    class Mode;
}

class QTcpSocket;

namespace GrumpyIRC
{
    class LIBCORESHARED_EXPORT GP : public QObject
    {
            Q_OBJECT
        public:
            GP();
            virtual ~GP();
            virtual bool IsConnected() const;
            void SendPacket(QHash<QString, QVariant> packet);

        signals:
            void Event_Connected();
            void Event_Disconnected();
            void Event_Timeout();

        public slots:
            void OnPing();
            void OnPingSend();
            void OnError(QAbstractSocket::SocketError er);
            void OnReceive();
            void OnConnected();

        private:
            QTcpSocket *socket;

    };
}

#endif // GP_H
