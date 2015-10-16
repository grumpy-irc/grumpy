//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef GRUMPYDSESSION_H
#define GRUMPYDSESSION_H

#include "libcore_global.h"
#include "gp.h"
#include <QMutex>
#include <QObject>
#include <QString>
#include <QHash>
#include <QAbstractSocket>
#include <QTimer>

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
    class Scrollback;

    class LIBCORESHARED_EXPORT GrumpydSession : public GP
    {
            Q_OBJECT
        public:
            GrumpydSession(QString Hostname, QString UserName, QString Pass, int Port = GP_DEFAULT_PORT);
            virtual ~GrumpydSession();
            void Connect();

        signals:
            void Event_IncomingData(QByteArray data);

        public slots:
            void OnConnected();
            void OnDisconnect();
            void OnTimeout();

        private:
            bool SSL;
            QString hostname;
            QString username;
            QString password;
            int port;

    };
}

#endif // GRUMPYDSESSION_H
