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
#include "../libgp/gp.h"
#include "definitions.h"
#include "../libirc/libirc/serveraddress.h"
#include "networksession.h"
#include <QMutex>
#include <QObject>
#include <QtNetwork>
#include <QString>
#include <QHash>
#include <QAbstractSocket>
#include <QTimer>

#define GRUMPY_UL_INSERT 2
#define GRUMPY_UL_UPDATE 3
#define GRUMPY_UL_REMOVE 4

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
    class IRCSession;

    class LIBCORESHARED_EXPORT GrumpydSession : public NetworkSession
    {
            Q_OBJECT
        public:
            static QMutex Sessions_Lock;
            static QList<GrumpydSession*> Sessions;

            GrumpydSession(Scrollback *System, QString Hostname, QString UserName, QString Pass, int Port = GP_DEFAULT_PORT, bool ssl = false);
            virtual ~GrumpydSession();
            Scrollback *GetSystemWindow();
            virtual void Open(libirc::ServerAddress server);
            bool IsConnected() const;
            void SendMessage(Scrollback *window, QString text);
            libircclient::Network *GetNetwork(Scrollback *window = NULL);
            void SendRaw(Scrollback *window, QString raw);
            SessionType GetType();
            void SendAction(Scrollback *window, QString text);
            void RequestRemove(Scrollback *window);
            QList<QString> GetChannels(Scrollback *window);
            void RequestDisconnect(Scrollback *window, QString reason, bool auto_delete);
            void RequestPart(Scrollback *window);
            libircclient::Channel *GetChannel(Scrollback *window);
            Scrollback *GetScrollback(scrollback_id_t original_id);
            void RequestBL(Scrollback *window, scrollback_id_t from, unsigned int size);
            IRCSession *GetSession(unsigned int nsid);
            QString GetLocalUserModeAsString(Scrollback *window);
            IRCSession *GetSessionFromWindow(Scrollback *scrollback);
            void Connect();
            libircclient::User *GetSelfNetworkID(Scrollback *window);
            unsigned long long GetBytesRcvd();
            unsigned long long GetBytesSent();

        signals:
            void Event_IncomingData(QByteArray data);
            //void Event_Deleted();

        public slots:
            void OnSslHandshakeFailure(QList<QSslError> errors, bool *ok);
            void OnDisconnect();
            void OnTimeout();
            void OnConnected();
            void OnIncomingCommand(gp_command_t text, QHash<QString, QVariant> parameters);

        private:
            void processNewScrollbackItem(QHash<QString, QVariant> hash);
            void processNetwork(QHash<QString, QVariant> hash);
            void processULSync(QHash<QString, QVariant> hash);
            void processNetworkResync(QHash<QString, QVariant> hash);
            void processChannel(QHash<QString, QVariant> hash);
            void processNick(QHash<QString, QVariant> hash);
            void processRequest(QHash<QString, QVariant> hash);
            void processChannelResync(QHash<QString, QVariant> hash);
            void processSResync(QHash<QString, QVariant> parameters);
            void processPSResync(QHash<QString, QVariant> parameters);
            void closeError(QString error);
            libgp::GP *gp;
            //! Irc sessions associated with their ROOT window so that we can figure out the network just from parent window
            QHash<Scrollback*, IRCSession*> sessionList;
            //! This is a persistent storage which contains all scrollbacks that are meant to belong to this grumpyd
            //! it's used to fasten up the resolution of scrollbacks by the original id
            QHash<scrollback_id_t, Scrollback*> scrollbackHash;
            Scrollback *systemWindow;
            bool SSL;
            QString hostname;
            QString username;
            QString password;
            int port;
    };
}

#endif // GRUMPYDSESSION_H
