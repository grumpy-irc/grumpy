//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef GDUSER_H
#define GDUSER_H

#include "../libcore/definitions.h"

#include <QList>
#include <QString>
#include "../libirc/libirc/serveraddress.h"

namespace GrumpyIRC
{
    class VirtualScrollback;
    class Scrollback;
    class Session;
    class UserConf;
    class Role;
    class SyncableIRCSession;

    class User
    {
        public:
            static QString EncryptPw(QString Password);
            static QList<User*> UserInfo;
            /*!
             * \brief Login try to login user using the provided credentials, if login is successful a pointer is returned
             * \param user
             * \param pw
             * \return
             */
            static User *Login(QString user, QString pw);
            static User *GetUser(user_id_t uid);
            static User *CreateUser(QString name, QString pass);
            static bool RemoveUser(user_id_t id);

            User(QString Name, QString Password, user_id_t User_ID);
            ~User();
            int GetSessionCount();
            int GetIRCSessionCount();
            void InsertSession(Session *sx);
            QString GetName() const;
            void RemoveSession(Session *sx);
            void RemoveIRCSession(SyncableIRCSession *session);
            SyncableIRCSession *ConnectToIRCServer(libirc::ServerAddress info);
            void RegisterSession(SyncableIRCSession *session);
            bool IsAuthorized(QString perm);
            //! Return true if this user is currently connected to grumpyd
            bool IsOnline();
            //! Return true if user is connected at least to 1 IRC server
            bool IsConnectedToIRC();
            QList<Session*> GetGPSessions() const;
            Session *GetAnyGPSession();
            QList<VirtualScrollback*> GetScrollbacks();
            QString GetPassword() const;
            VirtualScrollback *GetScrollback(scrollback_id_t id);
            SyncableIRCSession *GetSIRCSession(unsigned int sid);
            QList<SyncableIRCSession*> GetSIRCSessions();
            UserConf *GetConfiguration();
            //! Sends a RAW irc text to all IRC sessions that belong to this user
            void SendRawToIrcs(QString raw);
            Role *GetRole();
            user_id_t GetID();
            void SetRole(Role *rx);
            void RegisterScrollback(VirtualScrollback *scrollback, bool skip = false);
            bool IsLocked();
            void Lock();
            void Unlock();
            void Kick();
            QString DefaultNick;

        private:
            static user_id_t LastID;
            QHash<scrollback_id_t, VirtualScrollback*> scrollbacks;
            user_id_t id;
            Role *role;
            UserConf *conf;
            QList<Session*> sessions_gp;
            QList<SyncableIRCSession*> sessions;
            QString username;
            QString password;
            bool locked = false;
    };
}

#endif // USER_H
