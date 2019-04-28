//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SYNCABLEIRCSESSION_H
#define SYNCABLEIRCSESSION_H

#include "../libirc/libircclient/mode.h"
#include "../libcore/ircsession.h"

namespace GrumpyIRC
{
    class User;

    //! Override of standard IRCSession which is automagically syncing all its events with connected clients
    class SyncableIRCSession : public IRCSession
    {
            Q_OBJECT
        public:
            static void SetLastNID(unsigned int nid);
            static SyncableIRCSession *Open(Scrollback *system_window, libirc::ServerAddress &server, User *owner);

            SyncableIRCSession(QHash<QString, QVariant> sx, User *user, Scrollback *root = nullptr);
            SyncableIRCSession(Scrollback *system, User *user, Scrollback *root = nullptr);
            SyncableIRCSession(unsigned int id, Scrollback *system, User *user, QList<Scrollback*> sl);
            ~SyncableIRCSession() override = default;
            void Connect(libircclient::Network *Network) override;
            void Connect();
            void ResyncChannel(libircclient::Channel* channel);
            User *GetOwner() const;
            void ResyncChannel(libircclient::Channel *channel, QHash<QString, QVariant> cx);
            void Resync(QHash<QString, QVariant> network);
            void RequestDisconnect(Scrollback *scrollback, QString reason, bool auto_delete) override;
            void RegisterScrollback(Scrollback *scrollback);
            Scrollback *GetScrollbackForUser(QString user) override;
            void SetHostname(const QString &text);
            void SetName(const QString &text);
            void SetNick(const QString &text);
            void SetIdent(const QString &text);
            void SetSSL(bool is_ssl);
            void SetPort(unsigned int port);
        protected:
            Configuration *GetConfiguration() override;
            void SetDisconnected() override;
            void connInternalSocketSignals() override;
            void free() override;
        //signals:
        public slots:
            void OnIRCSelfJoin(libircclient::Channel *channel) override;
            void OnIRCJoin(libircclient::Parser *px, libircclient::User *user, libircclient::Channel *channel) override;
            void OnNICK(libircclient::Parser *px, QString old_, QString new_) override;
            void OnIRCSelfNICK(libircclient::Parser *px, QString previous, QString nick) override;
            void OnKICK(libircclient::Parser *px, libircclient::Channel *channel) override;
            void OnPart(libircclient::Parser *px, libircclient::Channel *channel) override;
            void OnConnectionFail(QAbstractSocket::SocketError er) override;
            void OnSelf_KICK(libircclient::Parser *px, libircclient::Channel *channel) override;
            void OnTOPIC(libircclient::Parser *px, libircclient::Channel *channel, QString previous_one) override;
            void OnQuit(libircclient::Parser *px, libircclient::Channel *channel) override;
            void OnSelfPart(libircclient::Parser *px, libircclient::Channel *channel) override;
            void OnTopicInfo(libircclient::Parser *px, libircclient::Channel *channel) override;
            void OnInfo(libircclient::Parser *px);
            void OnEndOfNames(libircclient::Parser *px) override;
            void OnUMODE(libircclient::Parser *px, libircclient::Channel *channel, libircclient::User *user) override;
            void OnWHO(libircclient::Parser *px, libircclient::Channel *channel, libircclient::User *user) override;
            void OnPModeInsert(libircclient::Parser *px, libircclient::ChannelPMode mode, libircclient::Channel *channel);
            void OnPModeRemove(libircclient::Parser *px, libircclient::ChannelPMode mode, libircclient::Channel *channel);
            void OnMODE(libircclient::Parser *px) override;
            void OnMODEInfo(libircclient::Parser* px, libircclient::Channel *channel) override;
            void OnChannelMODE(libircclient::Parser *px, libircclient::Channel *channel) override;
            void OnUserAwayStatusChange(libircclient::Parser *px, libircclient::Channel *ch, libircclient::User *ux) override;
            void OnServer_ISUPPORT(libircclient::Parser *px);
            void OnMessage(libircclient::Parser *px) override;
        private:
            void post_init();
            void rmScrollback(Scrollback *scrollback) override;
            void resyncULRemove(libircclient::Channel *channel, QString user);
            void resyncUL(libircclient::Channel *channel, int mode, libircclient::User *user);
            //! This is a cache of nicknames that we recently sent AFK message to, we keep last time of message to prevent spam
            QHash<QString, QDateTime> targetAFKCache;
            //! User who owns this session
            User *owner;
    };
}

#endif // SYNCABLEIRCSESSION_H
