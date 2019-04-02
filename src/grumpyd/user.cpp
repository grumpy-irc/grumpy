//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include <QCryptographicHash>
#include <QStringList>
#include "corewrapper.h"
#include "user.h"
#include "security.h"
#include "grumpyconf.h"
#include "databasebackend.h"
#include "grumpyd.h"
#include "session.h"
#include "userconfiguration.h"
#include "virtualscrollback.h"
#include "../libcore/core.h"
#include "../libcore/eventhandler.h"
#include "syncableircsession.h"

using namespace GrumpyIRC;

user_id_t User::LastID = 0;
QList<User*> User::UserInfo;

QString User::EncryptPw(const QString& Password)
{
    return QString(QCryptographicHash::hash(Password.toUtf8(), QCryptographicHash::Md5).toHex());
}

User *User::Login(QString user, QString pw)
{
    user = user.toLower();
    foreach (User *ux, UserInfo)
    {
        if (ux->username.toLower() == user)
        {
            if (ux->password != pw)
                return nullptr;
            else
                return ux;
        }
    }

    return nullptr;
}

User *User::GetUser(user_id_t uid)
{
    foreach (User *user, UserInfo)
    {
        if (user->GetID() == uid)
            return user;
    }

    return nullptr;
}

User *User::CreateUser(QString name, QString pass, Role *role)
{
    user_id_t id = ++User::LastID;
    User *user = new User(name, pass, id);
    user->SetRole(role);
    User::UserInfo.append(user);
    Grumpyd::GetBackend()->StoreUser(user);
    GRUMPY_LOG("Created new user: " + name);
    return user;
}

bool User::RemoveUser(user_id_t id)
{
    User *user = User::GetUser(id);
    if (!user)
        return false;

    User::UserInfo.removeAll(user);
    Grumpyd::GetBackend()->RemoveUser(user);
    GRUMPY_LOG("Deleted user: " + user->GetName());
    delete user;
    return true;
}

bool User::IsValid(QString user)
{
    if (user.isEmpty())
        return false;
    if (user.length() > 128)
        return false;
    if (user.contains(" "))
        return false;
    if (user.contains("\n"))
        return false;
    if (user.contains("\t"))
        return false;
    if (user.contains("*"))
        return false;

    return true;
}

User::User(QString Name, QString Password, user_id_t User_ID, bool is_locked)
{
    this->username = Name;
    this->locked = is_locked;
    this->DefaultNick = "Grumpyd user";
    this->role = nullptr;
    this->id = User_ID;
    this->conf = new UserConf(User_ID);
    this->conf->Load();
    this->password = Password;
    if (this->id > User::LastID)
        User::LastID = this->id;
}

User::~User()
{
    delete this->conf;
}

int User::GetSessionCount()
{
    return this->sessions_gp.count();
}

int User::GetIRCSessionCount()
{
    return this->sessions.count();
}

void User::InsertSession(Session *sx)
{
    if (this->sessions_gp.isEmpty())
    {
        this->sessions_gp.append(sx);
        // This is a first session to connect, let's check if there is a hook
        GRUMPY_DEBUG("First session connected for " + this->GetName(), 1);
        if (this->conf->Contains("session_on_conn_raw"))
        {
            GRUMPY_DEBUG("Sending RAW data for session_on_conn_raw for user: " + this->GetName(), 1);
            this->SendRawToIrcs(this->conf->GetValueAsString("session_on_conn_raw"));
        }
        return;
    }
    this->sessions_gp.append(sx);
    // Someone just connected to this user account, let's remove the away status if set before by auto-away system
    // please keep in mind that by default GrumpyChat has 2 away systems: one that is based on first and last session
    // disconnect and then another one UI based controlled by clients, that is the one we care about here, the first
    // one was already handled in code above this.
    this->UpdateAway();
}

QString User::GetName() const
{
    return this->username;
}

void User::RemoveSession(Session *sx)
{
    this->sessions_gp.removeAll(sx);
    if (this->sessions_gp.isEmpty())
    {
        GRUMPY_DEBUG("Last session disconnected for " + this->GetName(), 1);
        if (this->conf->Contains("session_on_disc_raw"))
        {
            GRUMPY_DEBUG("Sending RAW data for session_on_disc_raw for user: " + this->GetName(), 1);
            this->SendRawToIrcs(this->conf->GetValueAsString("session_on_disc_raw"));
        }
    }
}

void User::RemoveIRCSession(SyncableIRCSession *session)
{
    this->sessions.removeAll(session);
}

void User::Shutdown()
{
    foreach (Session *session, this->sessions_gp)
        session->Shutdown();

    this->DisconnectAllIRCSessions();
}

void User::DisconnectAllGrumpySessions()
{
    foreach (Session *session, this->sessions_gp)
        session->Disconnect();
}

void User::DisconnectAllIRCSessions()
{
    foreach (SyncableIRCSession *session, this->sessions)
        session->RequestDisconnect(nullptr, this->GetConfiguration()->GetValueAsString("quit_message"), false);
}

void User::SetRole(Role *rx)
{
    this->role = rx;
}

void User::RegisterScrollback(VirtualScrollback *scrollback, bool skip)
{
    // We call this same function when recovering the user from db
    // in that case we must not store it as it's already in there
    if (!skip)
        Grumpyd::GetBackend()->StoreScrollback(this, (Scrollback*)scrollback);
    if (this->scrollbacks.contains(scrollback->GetOriginalID()))
        throw new Exception("This scrollback is already registered for user", BOOST_CURRENT_FUNCTION);
    this->scrollbacks.insert(scrollback->GetOriginalID(), scrollback);
}

bool User::IsLocked()
{
    return this->locked;
}

void User::Lock()
{
    this->locked = true;
    this->Kick();
    Grumpyd::GetBackend()->LockUser(this);
}

void User::Unlock()
{
    this->locked = false;
    Grumpyd::GetBackend()->UnlockUser(this);
}

void User::Kick()
{
    foreach (Session *session, this->sessions_gp)
    {
        session->Kick();
    }
}

QByteArray User::StorageGet(QString key)
{
    if (!this->storage.contains(key))
        return QByteArray();
    return this->storage[key];
}

bool User::StorageSet(QString key, QByteArray data)
{
    // Count current size of storage
    unsigned long long current_size = 0;
    foreach (QString item, this->storage.keys())
        current_size += this->storage[item].size();

    // If the current storage size + size of new data is higher than total allowed size, deny the request
    if (CONF->MaxPersonalStorageSize < current_size + data.size())
        return false;

    if (!this->storage.contains(key))
    {
        this->storage.insert(key, data);
        Grumpyd::GetBackend()->InsertStorage(this->id, key, data);
    }
    else
    {
        this->storage[key] = data;
        Grumpyd::GetBackend()->UpdateStorage(this->id, key, data);
    }

    return true;
}

void User::StorageDelete(QString key)
{
    if (!this->storage.contains(key))
        return;

    Grumpyd::GetBackend()->RemoveStorage(this->id, key);
    this->storage.remove(key);
}

QList<QString> User::StorageList()
{
    return this->storage.keys();
}

bool User::StorageContains(QString key)
{
    return this->storage.contains(key);
}

void User::StorageLoad()
{
    this->storage = Grumpyd::GetBackend()->GetStorage(this->id);
}

void User::UpdateAway()
{
    bool now_is_away = this->IsAway();
    QString reason = "I am currently away on all clients";
    if (!this->LastAwayReason.isEmpty())
        reason = this->LastAwayReason;
    if (this->isAway != now_is_away)
    {
        this->isAway = now_is_away;
        if (!this->IsConnectedToIRC())
            return;
        // Change away status on all IRC networks
        foreach (SyncableIRCSession *session, this->sessions)
        {
            if (!session->IsConnected())
                continue;
            if (now_is_away)
                session->SetAway(reason);
            else
                session->UnsetAway();
        }
    }
}

SyncableIRCSession *User::ConnectToIRCServer(libirc::ServerAddress info)
{
    Scrollback *system_window = CoreWrapper::GrumpyCore->NewScrollback(NULL, info.GetHost(), ScrollbackType_System);
    if (info.GetNick().isEmpty())
        info.SetNick(this->conf->GetValueAsString("nick", "GrumpydUser"));
    GRUMPY_DEBUG("User " + this->username + " is connecting to network " + info.GetHost() + " as " + info.GetNick(), 1);
    SyncableIRCSession *session = SyncableIRCSession::Open(system_window, info, this);
    session->AutomaticallyRetrieveBanList = this->conf->GetValueAsBool("session_AutomaticallyRetrieveBanList", true);
    this->sessions.append(session);
    return session;
}

void User::RegisterSession(SyncableIRCSession *session)
{
    this->sessions.append(session);
}

bool User::IsAuthorized(QString perm)
{
    if (this->locked)
        return false;
    if (!this->role)
        return false;

    return this->role->IsAuthorized(perm);
}

bool User::IsOnline()
{
    return this->sessions_gp.count() > 0;
}

bool User::IsAway()
{
    if (!this->IsOnline())
        return true;
    foreach (Session *session, this->sessions_gp)
    {
        if (!session->IsAway)
        {
            return false;
        }
    }
    return true;
}

bool User::IsConnectedToIRC()
{
    foreach (SyncableIRCSession *session, this->sessions)
    {
        if (session->IsConnected())
            return true;
    }
    return false;
}

QList<Session*> User::GetGPSessions() const
{
    return this->sessions_gp;
}

QList<SyncableIRCSession *> User::GetSIRCSessions()
{
    return this->sessions;
}

UserConf *User::GetConfiguration()
{
    return this->conf;
}

void User::SendRawToIrcs(QString raw)
{
    if (raw.isEmpty())
        return;
    foreach (SyncableIRCSession *session, this->sessions)
    {
        if (!session->IsConnected())
        {
            // We can probably safely ignore this
            // let's go next
            continue;
        }
        foreach (QString line, raw.split("\n"))
        {
            if (line.startsWith("#"))
                continue;
            if (line.isEmpty())
                continue;
            session->SendRaw(session->GetSystemWindow(), line);
        }
    }
}

Role *User::GetRole()
{
    return this->role;
}

user_id_t User::GetID()
{
    return this->id;
}

SyncableIRCSession *User::GetSIRCSession(unsigned int sid)
{
    foreach (SyncableIRCSession *xx, this->sessions)
    {
        if (xx->GetSID() == sid)
            return xx;
    }

    return nullptr;
}

Session *User::GetAnyGPSession()
{
    if (!this->sessions_gp.count())
        return nullptr;

    // Return first session
    return this->sessions_gp.at(0);
}

QList<VirtualScrollback *> User::GetScrollbacks()
{
    return this->scrollbacks.values();
}

QString User::GetPassword() const
{
    return this->password;
}

VirtualScrollback *User::GetScrollback(scrollback_id_t id)
{
    if (this->scrollbacks.contains(id))
        return this->scrollbacks[id];

    // There is no such a scrollback with this id
    return nullptr;
}

