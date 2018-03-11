//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef IRCSESSION_H
#define IRCSESSION_H

#include "definitions.h"
#include <QMutex>
#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QString>
#include <QAbstractSocket>
#include "../libirc/libirc/serializableitem.h"
#include "../libirc/libircclient/network.h"
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
    class Configuration;
    class GrumpydSession;

    class LIBCORESHARED_EXPORT NetworkSniffer_Item
    {
        public:
            NetworkSniffer_Item(QByteArray data, bool is_outgoing);
            bool _outgoing;
            QDateTime Time;
            QString Text;
    };

    class LIBCORESHARED_EXPORT IRCSession : public NetworkSession, public libirc::SerializableItem
    {
            Q_OBJECT
            friend class GrumpydSession;
        public:
            static void Exit(QString message);
            static IRCSession *Open(Scrollback *system_window, libirc::ServerAddress &server, QString network = "", QString nick = "",
                                    QString ident = "", QString username = "", libircclient::Encoding network_enc = libircclient::EncodingDefault);
            static QMutex Sessions_Lock;
			static QList<IRCSession*> Sessions;

            IRCSession(QHash<QString, QVariant> sx, Scrollback *root = NULL);
			
            /*!
             * \brief IRCSession Creates a new uninitialized session, you should always create new sessions
             *                   with IRCSession::Open() instead of calling this directly
             */
            IRCSession(Scrollback *system, Scrollback *root = NULL);
            IRCSession(unsigned int id, Scrollback *system, Scrollback *root = NULL);
            virtual ~IRCSession();
            bool IsAway(Scrollback *scrollback = NULL);
            virtual Scrollback *GetSystemWindow();
            //! Return a first scrollback that matches the name, keep in mind that scrollbacks may have same name, for unique
            //! instance use SID of the give scrollback
            virtual Scrollback *GetScrollback(QString name);
            virtual Scrollback *GetScrollback(scrollback_id_t sid);
            virtual Scrollback *GetScrollbackByOriginal(scrollback_id_t original_sid);
            //! Get a scrollback for given channel, if it doesn't exist it returns NULL
            virtual Scrollback *GetScrollbackForChannel(QString channel);
            //! Retrieves a scrollback for given user, if it doesn't exist it will be created
            virtual Scrollback *GetScrollbackForUser(QString user);
            virtual libircclient::Network *GetNetwork(Scrollback *window = 0);
            virtual QList<Scrollback*> GetScrollbacks();
            virtual QList<Scrollback*> GetUserScrollbacks();
            virtual QList<Scrollback*> GetChannelScrollbacks();
            virtual unsigned int GetSID();
            virtual void Connect(libircclient::Network *Network);
            void SendMessage(Scrollback *window, QString target, QString text);
            void SendNotice(Scrollback *window, QString target, QString text);
            void SendMessage(Scrollback *window, QString text);
            void SendCTCP(Scrollback *window, QString target, QString ctcp, QString param);
            virtual bool IsConnected() const;
            virtual void SetNetwork(libircclient::Network *nt);
            void SendNotice(Scrollback *window, QString text);
            virtual QList<NetworkSniffer_Item*> GetSniffer();
            SessionType GetType();
            QList<QString> GetChannels(Scrollback *window);
            QHash<QString, QVariant> ToHash(int max_items = 2000);
            void LoadHash(QHash<QString, QVariant> hash);
            void SendAction(Scrollback *window, QString text);
            void SendRaw(Scrollback *window, QString raw, libircclient::Priority pr = libircclient::Priority_Normal);
            void RequestRemove(Scrollback *window);
            void RequestReconnect(Scrollback *window);
            void Query(Scrollback *window, QString target, QString message);
            libircclient::Channel *GetChannel(Scrollback *window);
            void RequestDisconnect(Scrollback *window, QString reason, bool auto_delete);
            void RequestPart(Scrollback *window);
            libircclient::User *GetSelfNetworkID(Scrollback *window);
            //! Used mostly only for synchronization with grumpyd
            virtual void RegisterChannel(libircclient::Channel *channel, Scrollback *window);
            void RetrieveChannelExceptionList(Scrollback *window, QString channel_name);
            void RetrieveChannelInviteList(Scrollback *window, QString channel_name);
            void RetrieveChannelBanList(Scrollback *window, QString channel_name);
            QString GetLocalUserModeAsString(Scrollback *window);
            QString GetName() const;
            QString GetHostname() const;
            QString GetNick() const;
            QString GetPassword() const;
            QString GetIdent() const;
            bool UsingSSL() const;
            unsigned int GetPort() const;
            QList<long long> GetPingHistory();
            bool IsAutoreconnect(Scrollback *window);
            void SetAutoreconnect(Scrollback *window, bool reconnect);
            QList<int> IgnoredNums;
            Scrollback *Root;
            bool AutomaticallyRetrieveBanList;
        signals:
            //! Emited when a new window for this session is open, needed by grumpyd for network sync
            void Event_ScrollbackIsOpen(Scrollback *window);
            void Event_ScrollbackIsClosed(Scrollback *window);
        protected slots:
            virtual void OnOutgoingRawMessage(QByteArray message);
            virtual void OnIncomingRawMessage(QByteArray message);
            virtual void OnConnectionFail(QAbstractSocket::SocketError er);
            virtual void OnDisconnect();
            virtual void OnMessage(libircclient::Parser *px);
            virtual void OnFailure(QString reason, int code);
            virtual void OnIRCJoin(libircclient::Parser *px, libircclient::User *user, libircclient::Channel *channel);
            virtual void OnUnknown(libircclient::Parser *px);
            virtual void OnNICK(libircclient::Parser *px, QString old_, QString new_);
            virtual void OnIRCSelfJoin(libircclient::Channel *channel);
            virtual void OnIRCSelfNICK(libircclient::Parser *px, QString previous, QString nick);
            virtual void OnKICK(libircclient::Parser *px, libircclient::Channel *channel);
            virtual void OnCTCP(libircclient::Parser *px, QString ctcp, QString pars);
            virtual void OnMOTD(libircclient::Parser *px);
            virtual void OnPart(libircclient::Parser *px, libircclient::Channel *channel);
            virtual void OnSelf_KICK(libircclient::Parser *px, libircclient::Channel *channel);
            virtual void OnTimeout();
            virtual void OnTOPIC(libircclient::Parser *px, libircclient::Channel *channel, QString previous_one);
            virtual void OnTOPICWhoTime(libircclient::Parser *px, libircclient::Channel *channel);
            virtual void OnNickConflict(libircclient::Parser *px);
            virtual void OnQuit(libircclient::Parser *px, libircclient::Channel *channel);
            virtual void OnSelfPart(libircclient::Parser *px, libircclient::Channel *channel);
            virtual void OnTopicInfo(libircclient::Parser *px, libircclient::Channel *channel);
            virtual void OnEndOfNames(libircclient::Parser *px);
            virtual void OnWHO(libircclient::Parser *px, libircclient::Channel *channel, libircclient::User *user);
            virtual void OnNotice(libircclient::Parser *px);
            virtual void OnWhoEnd(libircclient::Parser *px);
            virtual void OnMODEInfo(libircclient::Parser *px, libircclient::Channel *channel);
            virtual void OnMODETIME(libircclient::Parser *px);
            virtual void OnUpdateUserList();
            virtual void OnMODE(libircclient::Parser *px);
            virtual void OnUserAwayStatusChange(libircclient::Parser *px, libircclient::Channel *ch, libircclient::User *ux);
            virtual void OnChannelMODE(libircclient::Parser *px, libircclient::Channel *channel);
            virtual void OnUMODE(libircclient::Parser *px, libircclient::Channel *channel, libircclient::User *user);
            virtual void OnPMODE(libircclient::Parser *px, char mode);
            virtual void OnError(libircclient::Parser *px, QString error);
            virtual void OnEndOfBans(libircclient::Parser *px);
            virtual void OnEndOfInvites(libircclient::Parser *px);
            virtual void OnEndOfExcepts(libircclient::Parser *px);
            virtual void OnGeneric(libircclient::Parser *px);
            virtual void OnServerSideUnknown(libircclient::Parser *px);
            virtual void OnCapabilitiesNotSupported();
            virtual void OnINVITE(libircclient::Parser *px);
            virtual void OnWhoisUser(libircclient::Parser *px, libircclient::User *user);
            virtual void OnWhoisIdle(libircclient::Parser *px, unsigned int seconds_idle, QDateTime signon_time);
            virtual void OnWhoisOperator(libircclient::Parser *px);
            virtual void OnWhoisRegNick(libircclient::Parser *px);
            virtual void OnWhoisChannels(libircclient::Parser *px);
            virtual void OnWhoisHost(libircclient::Parser *px);
            virtual void OnWhoisEnd(libircclient::Parser *px);
            virtual void OnAway(libircclient::Parser *px);
            // Generic whois
            virtual void OnWhoisGen(libircclient::Parser *px);
            virtual void OnWhoisAcc(libircclient::Parser *px);
            virtual void OnPong(libircclient::Parser *px);

        protected:
            static unsigned int lastID;

            virtual void processME(libircclient::Parser *px, QString message);
            virtual void free();
            virtual void SetDisconnected();
            virtual void SetDead();
            //! Returns a configuration of grumpy, this method is overriden by grumpyd so that it returns
            //! the configuration for every user
            virtual Configuration *GetConfiguration();
            virtual void connInternalSocketSignals();
            //! This is only called by grumpy session, used for resync, pretty much just a performance tweaks
            //! so that we don't need to call GP_CMD_RESYNC_CHANNEL just for a simple nick change
            virtual void _gs_ResyncNickChange(QString new_, QString old_);
            virtual void rmWindow(Scrollback *window);
            virtual void SyncWindows(QHash<QString, QVariant> windows, QHash<QString, Scrollback*> *hash);
            QTimer timerUL;
            //! Sessions have unique ID that distinct them from sessions made to same irc network
            unsigned int SID;
            unsigned int _port;
            bool _ssl;
            QString _hostname;
            QString _name;
            QString _ident;
            QString _nick;
            QString _password;
            bool _autoReconnect;
            QList<NetworkSniffer_Item*> data;
            QList<QString> ignoringWho;
            QList<QString> ignoringBans;
            QList<QString> ignoringExceptions;
            QList<QString> ignoringInvites;
            QHash<QString, Scrollback*> channels;
            bool snifferEnabled;
            Scrollback *highlightCollector;
            int ulistUpdateTime;
            unsigned int maxSnifferBufferSize;
            libircclient::Network *network;
            QHash<QString, Scrollback*> users;
            Scrollback *systemWindow;
            QList<long long> pingHistory;
        private:
            bool isRetrievingWhoInfo(QString channel);
            void init(bool preindexed);
            void whoisIs(libircclient::Parser *parser);
            QList<QString> retrievingWho;
    };
}

#endif // IRCSESSION_H
