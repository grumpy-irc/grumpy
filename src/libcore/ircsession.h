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
#include <QDateTime>
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

    class LIBCORESHARED_EXPORT IRCSession : public QObject, public NetworkSession, public libirc::SerializableItem
    {
            Q_OBJECT
            friend class GrumpydSession;
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
            //! Return a first scrollback that matches the name, keep in mind that scrollbacks may have same name, for unique
            //! instance use SID of the give scrollback
            virtual Scrollback *GetScrollback(QString name);
            virtual Scrollback *GetScrollback(unsigned long long sid);
            virtual Scrollback *GetScrollbackByOriginal(unsigned long long original_sid);
            //! Get a scrollback for given channel, if it doesn't exist it returns NULL
            virtual Scrollback *GetScrollbackForChannel(QString channel);
            //! Retrieves a scrollback for given user, if it doesn't exist it will be created
            virtual Scrollback *GetScrollbackForUser(QString user);
            virtual libircclient::Network *GetNetwork();
            virtual unsigned int GetSID();
            virtual void Connect(libircclient::Network *Network);
            virtual void SendMessage(Scrollback *window, QString text);
            virtual bool IsConnected() const;
            virtual QList<NetworkSniffer_Item*> GetSniffer();
            SessionType GetType();
            QList<QString> GetChannels(Scrollback *window);
            QHash<QString, QVariant> ToHash(int max_items = 2000);
            void LoadHash(QHash<QString, QVariant> hash);
            void SendAction(Scrollback *window, QString text);
            void SendRaw(Scrollback *window, QString raw);
            void RequestRemove(Scrollback *window);
            void RequestDisconnect(Scrollback *window, QString reason, bool auto_delete);
            void RequestPart(Scrollback *window);
            //! Used mostly only for synchronization with grumpyd
            virtual void RegisterChannel(libircclient::Channel *channel, Scrollback *window);
            Scrollback *Root;
        signals:
            //! Emited when a new window for this session is open, needed by grumpyd for network sync
            void Event_ScrollbackIsOpen(Scrollback *window);
            void Event_ScrollbackIsClosed(Scrollback *window);
        protected slots:
            virtual void OnOutgoingRawMessage(QByteArray message);
            virtual void OnIncomingRawMessage(QByteArray message);
            virtual void OnConnectionFail(QAbstractSocket::SocketError er);
            virtual void OnMessage(libircclient::Parser *px);
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
            virtual void OnTOPIC(libircclient::Parser *px, libircclient::Channel *channel, QString previous_one);
            virtual void OnQuit(libircclient::Parser *px, libircclient::Channel *channel);
            virtual void OnSelfPart(libircclient::Parser *px, libircclient::Channel *channel);
            virtual void OnTopicInfo(libircclient::Parser *px, libircclient::Channel *channel);
            virtual void OnEndOfNames(libircclient::Parser *px);
            virtual void OnNotice(libircclient::Parser *px);
        protected:
            static unsigned int lastID;

            virtual void processME(libircclient::Parser *px, QString message);
            //! Returns a configuration of grumpy, this method is overriden by grumpyd so that it returns
            //! the configuration for every user
            virtual Configuration *GetConfiguration();
            //! This is only called by grumpy session, used for resync, pretty much just a performance tweaks
            //! so that we don't need to call GP_CMD_RESYNC_CHANNEL just for a simple nick change
            void _gs_ResyncNickChange(QString new_, QString old_);
            void rmWindow(Scrollback *window);
            void SyncWindows(QHash<QString, QVariant> windows, QHash<QString, Scrollback*> *hash);
            //! Sessions have unique ID that distinct them from sessions made to same irc network
            unsigned int SID;
            QList<NetworkSniffer_Item*> data;
            QHash<QString, Scrollback*> channels;
            libircclient::Network *network;
            QHash<QString, Scrollback*> users;
            Scrollback *systemWindow;
    };
}

#endif // IRCSESSION_H
