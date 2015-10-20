//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QHash>
#include <QThread>
#include <QList>
#include <QTcpSocket>
#include <QString>
#include <QMutex>
#include "../libcore/gp.h"

namespace GrumpyIRC
{
    class Session : public QThread
    {
            enum State
            {
                State_Login,
                State_Open,
                State_Killing,
                State_Exiting,
                State_Offline
            };

            Q_OBJECT
        public:
            static QList<Session*> Sessions();

            Session(qintptr SocketPtr);
            ~Session();
            void run();
            unsigned long GetSID();
            State SessionState;

        public slots:
            void OnCommand(QString text, QHash<QString, QVariant> parameters);

        signals:
            void OnError(int error, QString text);

        private:
            QTcpSocket *socket;
            GP *protocol;
            static unsigned long lSID;
            static QList<Session*> SessionList;
            static QMutex *sessions_lock;

            unsigned long SID;
    };
}

#endif // SESSION_H
