//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef GRUMPYDSESSION_H
#define GRUMPYDSESSION_H

#include "libcore_global.h"
#include "../libgp/gp.h"
#include "grumpyobject.h"
#include "definitions.h"
#include "../libirc/libirc/serveraddress.h"
#include "sniffer.h"
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
#define GP_ENOSPACE              -6
#define GP_ESSLHANDSHAKEFAILED   -20
#define GP_EIRCNOTCONN           -21
#define GP_ENOUSER               -40
#define GP_EWRONGUSER            -41
#define GP_ESELFTARGET           -42
#define GP_ENOCHANGE             -43
#define GP_ELOCKED               -44

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
#define GP_CMD_CHANNEL_JOIN                14 //"CHANNEL_JOIN"
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
#define GP_CMD_RESYNC_SCROLLBACK_PB        29 // Used to resync the scrollback's property bag - only change or append new items, doesn't clear existing ones
#define GP_CMD_OVERRIDE_SCROLLBACK_PB      30 // Replaces the property bag with another hash
#define GP_CMD_QUERY                       31
#define GP_CMD_STORAGE_SET                 32 // Used to write data to personal BLOB storage
#define GP_CMD_STORAGE_GET                 33 // Used to read data from personal BLOB storage
#define GP_CMD_STORAGE_DEL                 34
#define GP_CMD_AWAY                        35 // Change status
#define GP_CMD_HIDE_SB                     36 // Toggle hide status of a scrollback
#define GP_CMD_SYS_LIST_USER               40 // List grumpyd users
#define GP_CMD_SYS_CREATE_USER             41 // Create a new user
#define GP_CMD_SYS_REMOVE_USER             42 // Removes a user
#define GP_CMD_SYS_LOCK_USER               43
#define GP_CMD_SYS_UNLOCK_USER             44
#define GP_CMD_SYS_GRANT_ROLE              45
#define GP_CMD_SYS_REVOKE_ROLE             46
#define GP_CMD_SYS_ALTER_USER              47
#define GP_CMD_SYS_INSTALL_SCRIPT          48
#define GP_CMD_SYS_UNINST_SCRIPT           49
#define GP_CMD_SYS_LIST_SCRIPT             50
#define GP_CMD_GET_SNIFFER                 51
#define GP_CMD_SYS_RELOAD_SCRIPT           52
#define GP_CMD_SYS_RELOAD_ALL_SCRIPTS      53
#define GP_CMD_SYS_READ_SCRIPT_SOURCE_CODE 54
#define GP_CMD_SEARCH                      60

// PACKET VERIFICATION
// This system is used to verify if packet was delivered or not
#define GP_CMD_ACK                         200

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

    class LIBCORESHARED_EXPORT GrumpydSession : public NetworkSession, public GrumpyObject
    {
            Q_OBJECT
        public:
            static QMutex Sessions_Lock;
            static QList<GrumpydSession*> Sessions;

            GrumpydSession(Scrollback *System, const QString &Hostname, const QString &UserName, const QString &Pass, int Port = GP_DEFAULT_PORT, bool ssl = false);
             ~GrumpydSession() override;
            libircclient::Channel *GetChannel(Scrollback *scrollback) override;
            Scrollback *GetScrollback(scrollback_id_t original_id);
            IRCSession *GetSessionFromWindow(Scrollback *scrollback);
            Scrollback *GetSystemWindow() override;
            QList<QString> GetChannels(Scrollback *scrollback) override;
            SessionType GetType() override;
            bool IsAway(Scrollback *scrollback = nullptr) override;
            libircclient::Network *GetNetwork(Scrollback *scrollback = nullptr) override;
            virtual void Open(libirc::ServerAddress server);
            bool IsConnected() const override;
            void SendMessage(Scrollback *scrollback, QString text) override;
            void SendRaw(Scrollback *scrollback, QString raw, libircclient::Priority pr = libircclient::Priority_Normal) override;
            void SendAction(Scrollback *scrollback, QString text) override;
            void SendNotice(Scrollback *scrollback, QString text) override;
            void SendMessage(Scrollback *scrollback, QString target, QString message) override;
            void SendCTCP(Scrollback *scrollback, QString target, QString ctcp, QString param) override;
            void SendNotice(Scrollback *scrollback, QString target, QString message) override;
            void SendProtocolCommand(unsigned int command);
            void SendProtocolCommand(unsigned int command, const QHash<QString, QVariant> &parameters);
            IRCSession *GetSession(unsigned int nsid);
            QString GetLocalUserModeAsString(Scrollback *scrollback) override;
            void RetrieveChannelBanList(Scrollback *scrollback, QString channel_name) override;
            void RequestRemove(Scrollback *scrollback) override;
            void RequestHide(Scrollback *scrollback, bool hide);
            void RequestDisconnect(Scrollback *scrollback, QString reason, bool auto_delete) override;
            void ResyncPB(Scrollback *scrollback);
            void ResyncSingleItemPB(Scrollback *scrollback, const QString &name);
            void RequestPart(Scrollback *scrollback) override;
            void Query(Scrollback *scrollback, QString target, QString message) override;
            void RequestBL(Scrollback *scrollback, scrollback_id_t from, unsigned int size);
            void RequestReconnect(Scrollback *scrollback) override;
            void Connect();
            bool IsAutoreconnect(Scrollback *scrollback) override;
            void SetAutoreconnect(Scrollback *scrollback, bool reconnect) override;
            void SetAway(QString reason) override;
            void UnsetAway() override;
            void RequestSniffer(IRCSession *session);
            libircclient::User *GetSelfNetworkID(Scrollback *scrollback) override;
            QDateTime GetLastSnifferUpdate(IRCSession *session);
            QList<NetworkSniffer_Item> GetSniffer(IRCSession *session);
            unsigned long long GetCompressedBytesRcvd();
            unsigned long long GetCompressedBytesSent();
            unsigned long long GetBytesRcvd();
            unsigned long long GetBytesSent();
            unsigned long long GetPacketsSent();
            unsigned long long GetPacketsRcvd();
            bool IsReceivingLargePacket();
            qint64 GetReceivingPacketSize();
            qint64 GetProgress();
            //! Return last time when user list was updated
            QDateTime GetLastUpdateOfUserList();
            QDateTime GetLastUpdateOfScripts();
            //! Return user list from cache
            QList<QVariant> GetUserList();
            QList<QString> GetRoles();
            QString Version;
            bool IsOpening = false;
            QHash<QString, QVariant> Preferences;
            QList<QVariant> ScriptList;

        signals:
            void Event_IncomingData(QByteArray data);
            void Event_ScriptDeleted(const QHash<QString, QVariant> &info);
            void Event_ScriptInstalled(const QHash<QString, QVariant> &info);
            void Event_ScriptList(const QList<QVariant> &scripts);
            void Event_ScriptReloaded(const QHash<QString, QVariant> &info);
            void Event_ScriptsReloaded(const QHash<QString, QVariant> &info);
            void Event_ScriptSource(const QHash<QString, QVariant> &info);
            void Event_Error(const QHash<QString, QVariant> &info);
            //void Event_Deleted();

        public slots:
            void OnSslHandshakeFailure(QList<QSslError> errors, bool *ok);
            void OnDisconnect();
            void OnTimeout();
            void OnConnected();
            void OnError(const QString &reason, int num);
            void OnIncomingCommand(gp_command_t text, const QHash<QString, QVariant> &parameters);

        private:
            void kill();
            void processNewScrollbackItem(const QHash<QString, QVariant> &hash);
            void processNetwork(const QHash<QString, QVariant> &hash);
            void processULSync(const QHash<QString, QVariant> &hash);
            void processNetworkResync(const QHash<QString, QVariant> &hash);
            void processChannel(const QHash<QString, QVariant> &hash);
            void processNick(const QHash<QString, QVariant> &hash);
            void processPreferences(const QHash<QString, QVariant> &hash);
            void processChannelModeSync(const QHash<QString, QVariant> &hash);
            void processLScript(const QHash<QString, QVariant> &hash);
            void processIScript(const QHash<QString, QVariant> &hash);
            void processUScript(const QHash<QString, QVariant> &hash);
            void processRequest(const QHash<QString, QVariant> &hash);
            void processChannelResync(const QHash<QString, QVariant> &hash);
            void processSResync(const QHash<QString, QVariant> &parameters);
            void processPBResync(const QHash<QString, QVariant> &parameters);
            void processAck(const QHash<QString, QVariant> &parameters);
            void processPSResync(const QHash<QString, QVariant> &parameters);
            void processRemove(const QHash<QString, QVariant> &parameters);
            void processSearch(const QHash<QString, QVariant> &parameters);
            void processUserList(const QHash<QString, QVariant> &parameters);
            void processSniffer(const QHash<QString, QVariant> &parameters);
            void processHello(const QHash<QString, QVariant> &parameters);
            void processLoginOK(const QHash<QString, QVariant> &parameters);
            void processInit(const QHash<QString, QVariant> &parameters);
            void processQuit(const QHash<QString, QVariant> &parameters);
            void processRefuse(const QHash<QString, QVariant> &parameters);
            void processScriptSource(const QHash<QString, QVariant> &parameters);
            void processErr(const QHash<QString, QVariant> &parameters);
            void processHideSB(const QHash<QString, QVariant> &parameters);
            void freememory();
            void closeError(const QString &error);
            QHash<unsigned int, QList<NetworkSniffer_Item>> snifferCache;
            QHash<unsigned int, QDateTime> snifferCacheLastUpdate;
            bool AutoReconnect;
            QDateTime syncInit;
            QList<unsigned int> processedMessages;
            QDateTime lastScriptListUpdate;
            unsigned int lastProcessedMessage = 1;
            bool syncing;
            libgp::GP *gp;
            //! Irc sessions associated with their ROOT scrollback so that we can figure out the network just from parent scrollback
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
            // Buffer only
            QList<QVariant> userList;
            QList<QString> roles;
            // Used for comparisons
            QDateTime lastUserListUpdate;
    };
}

#endif // GRUMPYDSESSION_H
