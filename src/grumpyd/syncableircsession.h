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

#include "../libcore/ircsession.h"

namespace GrumpyIRC
{
    class User;

    //! Override of standard IRCSession which is automagically syncing all its events with connected clients
    class SyncableIRCSession : public IRCSession
    {
            Q_OBJECT
        public:
            static SyncableIRCSession *Open(Scrollback *system_window, libirc::ServerAddress &server, User *owner);

            SyncableIRCSession(QHash<QString, QVariant> sx, User *user, Scrollback *root = NULL);
            SyncableIRCSession(Scrollback *system, User *user, Scrollback *root = NULL);
            void Connect(libircclient::Network *Network);
            void ResyncChannel(libircclient::Channel* channel);
            void RequestDisconnect(Scrollback *window, QString reason, bool auto_delete);
            ~SyncableIRCSession();
        protected:
            Configuration *GetConfiguration();
        //signals:
        public slots:
            void OnIRCSelfJoin(libircclient::Channel *channel);
            void OnIRCJoin(libircclient::Parser *px, libircclient::User *user, libircclient::Channel *channel);
            void OnNICK(libircclient::Parser *px, QString old_, QString new_);
            void OnIRCSelfNICK(libircclient::Parser *px, QString previous, QString nick);
            void OnKICK(libircclient::Parser *px, libircclient::Channel *channel);
            void OnPart(libircclient::Parser *px, libircclient::Channel *channel);
            void OnSelf_KICK(libircclient::Parser *px, libircclient::Channel *channel);
            void OnTOPIC(libircclient::Parser *px, libircclient::Channel *channel, QString previous_one);
            void OnQuit(libircclient::Parser *px, libircclient::Channel *channel);
            void OnSelfPart(libircclient::Parser *px, libircclient::Channel *channel);
            void OnTopicInfo(libircclient::Parser *px, libircclient::Channel *channel);
            void OnInfo(libircclient::Parser *px);
            void OnEndOfNames(libircclient::Parser *px);
        private:
            void resyncULRemove(libircclient::Channel *channel, QString user);
            void resyncUL(libircclient::Channel *channel, int mode, libircclient::User *user);
            //! User who owns this session
            User *owner;
    };
}

#endif // SYNCABLEIRCSESSION_H
