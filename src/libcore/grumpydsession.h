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

#define GP_MESSAGETYPE_NORMAL 1
#define GP_MESSAGETYPE_ACTION 2
#define GP_MESSAGETYPE_NOTICE 3
#define GP_MESSAGETYPE_ISCTCP 4

#define GP_EALREADYLOGGEDIN      -1
#define GP_EINVALIDLOGINPARAMS   -2
#define GP_ENOSERVER             -3
#define GP_ENETWORKNOTFOUND      -4
#define GP_ESCROLLBACKNOTFOUND   -5
#define GP_ESSLHANDSHAKEFAILED   -20
#define GP_EIRCNOTCONN           -21

#define GP_CMD_HELLO                        1 //"HELLO"
#define GP_CMD_SERVER                       2 //"SERVER"
#define GP_CMD_NETWORK_INFO                 3 //"NETWORK_INFO"
//! When nickname was changed this requires update of network structure
#define GP_CMD_NICK                         4 //"NICK"
#define GP_CMD_LOGIN                        5 //"LOGIN"
#define GP_CMD_LOGIN_OK                     6 //"LOGIN_OK"
#define GP_CMD_LOGIN_FAIL                   7 //"LOGIN_FAIL"
#define GP_CMD_RAW                          8 //"RAW"
#define GP_CMD_PERMDENY                     9 //"PERMDENY"
#define GP_CMD_UNKNOWN                     10 //"UNKNOWN"
#define GP_CMD_MESSAGE                     11 //"MESSAGE"
#define GP_CMD_ERROR                       12 //"ERROR"
#define GP_CMD_IRC_QUIT                    13 //"IRC_QUIT"
//! This command is delivered when a new channel is joined by user
#define GP_CMD_CHANNEL_JOIN                14 //"CHANNEL_JOIN"`
#define GP_CMD_CHANNEL_RESYNC              15 //"CHANNEL_RESYNC"
//! On resync of a whole network, this only involves internal network parameters
//! not channel lists and other lists of structures that have pointers
#define GP_CMD_NETWORK_RESYNC              16 //"NETWORK_RESYNC"
#define GP_CMD_SCROLLBACK_RESYNC           17 //"SCROLLBACK_RESYNC"
//! Used to save traffic on events where we need to resync some of the scrollback attributes but not buffer contents
//! only resync some of the scrollback items
#define GP_CMD_SCROLLBACK_PARTIAL_RESYNC   18 //"SCROLLBACK_PARTIAL_RESYNC"
#define GP_CMD_SCROLLBACK_LOAD_NEW_ITEM    19 //"SCROLLBACK_LOAD_NEW_ITEM"
#define GP_CMD_OPTIONS                     20 //"OPTIONS"
#define GP_CMD_USERLIST_SYNC               21 //"USERLIST_SYNC"
#define GP_CMD_REQUEST_ITEMS               22 //"REQUEST_ITEMS"
#define GP_CMD_REGISTER                    23
#define GP_CMD_INIT                        24
#define GP_CMD_RECONNECT                   25
#define GP_CMD_REMOVE                      26
#define GP_CMD_REQUEST_INFO                27
#define GP_CMD_RESYNC_MODE                 28

#define GP_MODETYPE_UMODE 1
#define GP_MODETYPE_PMODE 2
#define GP_MODETYPE_CMODE 3
#define GP_MODETYPE_RMODE 4
#define GP_MODETYPE_SMODE 5
#define GP_MODETYPE_KMODE 6

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
            libircclient::Channel *GetChannel(Scrollback *window);
            Scrollback *GetScrollback(scrollback_id_t original_id);
            IRCSession *GetSessionFromWindow(Scrollback *scrollback);
            Scrollback *GetSystemWindow();
            QList<QString> GetChannels(Scrollback *window);
            SessionType GetType();
            bool IsAway(Scrollback *scrollback = NULL);
            libircclient::Network *GetNetwork(Scrollback *window = NULL);
            virtual void Open(libirc::ServerAddress server);
            bool IsConnected() const;
            void SendMessage(Scrollback *window, QString text);
            void SendRaw(Scrollback *window, QString raw);
            void SendAction(Scrollback *window, QString text);
            void SendNotice(Scrollback *window, QString text);
            void SendMessage(Scrollback *window, QString target, QString message);
            void SendCTCP(Scrollback *window, QString target, QString ctcp, QString param);
            void SendNotice(Scrollback *window, QString target, QString message);
            void SendProtocolCommand(unsigned int command, QHash<QString, QVariant> parameters);
            IRCSession *GetSession(unsigned int nsid);
            QString GetLocalUserModeAsString(Scrollback *window);
            void RetrieveChannelBanList(Scrollback *window, QString channel_name);
            void RequestRemove(Scrollback *window);
            void RequestDisconnect(Scrollback *window, QString reason, bool auto_delete);
            void RequestPart(Scrollback *window);
            void RequestBL(Scrollback *window, scrollback_id_t from, unsigned int size);
            void RequestReconnect(Scrollback *window);
            void Connect();
            bool IsAutoreconnect(Scrollback *window);
            void SetAutoreconnect(Scrollback *window, bool reconnect);
            libircclient::User *GetSelfNetworkID(Scrollback *window);
            unsigned long long GetCompressedBytesRcvd();
            unsigned long long GetCompressedBytesSent();
            unsigned long long GetBytesRcvd();
            unsigned long long GetBytesSent();
            unsigned long long GetPacketsSent();
            unsigned long long GetPacketsRcvd();
            QString Version;

        signals:
            void Event_IncomingData(QByteArray data);
            //void Event_Deleted();

        public slots:
            void OnSslHandshakeFailure(QList<QSslError> errors, bool *ok);
            void OnDisconnect();
            void OnTimeout();
            void OnConnected();
            void OnError(QString reason, int num);
            void OnIncomingCommand(gp_command_t text, QHash<QString, QVariant> parameters);

        private:
            void kill();
            void processNewScrollbackItem(QHash<QString, QVariant> hash);
            void processNetwork(QHash<QString, QVariant> hash);
            void processULSync(QHash<QString, QVariant> hash);
            void processNetworkResync(QHash<QString, QVariant> hash);
            void processChannel(QHash<QString, QVariant> hash);
            void processNick(QHash<QString, QVariant> hash);
            void processPreferences(QHash<QString, QVariant> hash);
            void processChannelModeSync(QHash<QString, QVariant> hash);
            void processRequest(QHash<QString, QVariant> hash);
            void processChannelResync(QHash<QString, QVariant> hash);
            void processSResync(QHash<QString, QVariant> parameters);
            void processPSResync(QHash<QString, QVariant> parameters);
            void processRemove(QHash<QString, QVariant> parameters);
            void freememory();
            void closeError(QString error);
            bool AutoReconnect;
            QDateTime syncInit;
            bool syncing;
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
