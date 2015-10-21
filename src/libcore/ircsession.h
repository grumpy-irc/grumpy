//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef IRCSESSION_H
#define IRCSESSION_H

#include <QMutex>
#include <QObject>
#include <QString>
#include <QAbstractSocket>
#include "../libirc/libirc/serializableitem.h"
#include "networksession.h"
#include "libcore_global.h"

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

namespace GrumpyIRC
{
    class Scrollback;

    class LIBCORESHARED_EXPORT IRCSession : public QObject, public NetworkSession, public libirc::SerializableItem
    {
            Q_OBJECT
        public:
            static IRCSession *Open(Scrollback *system_window, libirc::ServerAddress &server, QString network = "", QString nick = "",
                                    QString ident = "", QString username = "");
            static QMutex Sessions_Lock;
			static QList<IRCSession*> Sessions;

            IRCSession(QHash<QString, QVariant> sx, Scrollback *root = NULL);
			
            /*!
             * \brief IRCSession Creates a new uninitialized session, you should always create new sessions
             *                   with IRCSession::Open() instead of calling this directly
             */
            IRCSession(Scrollback *system, Scrollback *root = NULL);
            virtual ~IRCSession();
            virtual Scrollback *GetSystemWindow();
            libircclient::Network *GetNetwork();
            virtual void Connect(libircclient::Network *Network);
            void SendMessage(Scrollback *window, QString text);
            bool IsConnected() const;
            virtual Scrollback *GetScrollbackForChannel(QString channel);
            SessionType GetType();
            Scrollback *GetScrollbackForUser(QString user);
            QHash<QString, QVariant> ToHash();
            void LoadHash(QHash<QString, QVariant> hash);
            Scrollback *Root;
        private slots:
            void OnIncomingRawMessage(QByteArray message);
            void OnConnectionFail(QAbstractSocket::SocketError er);
            void OnMessage(libircclient::Parser *px);
            void OnIRCJoin(libircclient::Parser *px, libircclient::User *user, libircclient::Channel *channel);
            void OnUnknown(libircclient::Parser *px);
            void OnNICK(libircclient::Parser *px, QString old_, QString new_);
            void OnIRCSelfJoin(libircclient::Channel *channel);
            void OnIRCSelfNICK(libircclient::Parser *px, QString previous, QString nick);
            void OnKICK(libircclient::Parser *px, libircclient::Channel *channel);
            void OnPart(libircclient::Parser *px, libircclient::Channel *channel);
            void OnSelf_KICK(libircclient::Parser *px, libircclient::Channel *channel);
            void OnTOPIC(libircclient::Parser *px, libircclient::Channel *channel, QString previous_one);
            void OnQuit(libircclient::Parser *px, libircclient::Channel *channel);
            void OnSelfPart(libircclient::Parser *px, libircclient::Channel *channel);
            void OnTopicInfo(libircclient::Parser *px, libircclient::Channel *channel);
            void OnEndOfNames(libircclient::Parser *px);
            void OnNotice(libircclient::Parser *px);
        protected:
            QHash<QString, Scrollback*> channels;
            libircclient::Network *network;
            QHash<QString, Scrollback*> users;
            Scrollback *systemWindow;
    };
}

#endif // IRCSESSION_H
