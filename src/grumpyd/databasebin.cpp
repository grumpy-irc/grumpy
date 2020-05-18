//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "databasebin.h"
#include "user.h"
#include "virtualscrollback.h"
#include "grumpyconf.h"
#include "corewrapper.h"
#include "security.h"
#include "syncableircsession.h"
#include "session.h"
#include "../libcore/configuration.h"
#include "../libcore/core.h"
#include "../libcore/generic.h"
#include "../libcore/eventhandler.h"
#include "../libcore/exception.h"
#include <QFile>
#include <QByteArray>
#include <QDataStream>
#include <QRegExp>
#include <QDir>

using namespace GrumpyIRC;

static QByteArray ToArray(QHash<QString, QVariant> data)
{
    QByteArray result;
    QDataStream stream(&result, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_0);
    stream << data;
    return result;
}

static QByteArray ToArray(int number)
{
    QByteArray result;
    QDataStream stream(&result, QIODevice::ReadWrite);
    GP_INIT_DS(stream);
    stream << number;
    return result;
}

static QHash<QString, QVariant> hashFromArray(QByteArray *data)
{
    QHash<QString, QVariant> result;
    QDataStream stream(data, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_0);
    stream >> result;
    return result;
}

static int intFromArray(QByteArray *data)
{
    int result;
    QDataStream stream(data, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_0);
    stream >> result;
    return result;
}

DatabaseBin::DatabaseBin()
{
    this->PathPrefix = CONF->GetDatafilePath() + QDir().separator() + "database";
    GRUMPY_LOG("Storage path: " + this->PathPrefix);
    this->magicCode.insert("info", "GrumpyChat binary backend database");
    this->magicCode.insert("grumpy_version", CONF->GetConfiguration()->GetVersion());
    this->magicCode.insert("format_version_major", 1);
    this->magicCode.insert("format_version_minor", 0);
    this->magic = ToArray(this->magicCode);
    this->headerSize = ToArray(0).size();
}

DatabaseBin::~DatabaseBin()
{

}

void DatabaseBin::LoadRoles()
{
    Role::Defaults();
}

void DatabaseBin::LoadUsers()
{
    if (!QFile(this->GetGlobalConfigPath() + "users.dat").exists())
    {
        CONF->Init = true;
    } else
    {
        // Load the user DB
        QHash<QString, QVariant> user_db = this->loadSingleQHash(this->GetGlobalConfigPath() + "users.dat");
        foreach (QString user_id, user_db.keys())
        {
            QHash<QString, QVariant> user_info = user_db[user_id].toHash();
            User *user = new User(user_info["name"].toString(),
                                  user_info["pw"].toString(),
                                  user_info["id"].toUInt(),
                                  user_info["is_locked"].toBool());
            User::UserInfo.append(user);
            if (Role::Roles.contains(user_info["role"].toString()))
                user->SetRole(Role::Roles[user_info["role"].toString()]);
            user->StorageLoad();
        }
    }
}

QHash<QString, QVariant> DatabaseBin::GetConfiguration(user_id_t user)
{
    QHash<QString, QVariant> user_conf;

    return user_conf;
}

void DatabaseBin::SetConfiguration(user_id_t user, QHash<QString, QVariant> data)
{

}

void DatabaseBin::RemoveNetwork(IRCSession *session)
{

}

void DatabaseBin::RemoveScrollback(User *owner, Scrollback *sx)
{

}

QList<QVariant> DatabaseBin::FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size)
{
    QList<QVariant> results;

    return results;
}

QList<QVariant> DatabaseBin::Search(QString text, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

QList<QVariant> DatabaseBin::SearchRegular(QString regex, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

QList<QVariant> DatabaseBin::SearchOne(VirtualScrollback *scrollback, QString text, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

QList<QVariant> DatabaseBin::SearchOneRegular(VirtualScrollback *scrollback, QString regex, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

void DatabaseBin::UpdateRoles()
{

}

void DatabaseBin::StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item)
{

}

void DatabaseBin::LoadSessions()
{

}

void DatabaseBin::UpdateNetwork(IRCSession *session)
{

}

void DatabaseBin::RemoveUser(User *user)
{
    QString path = this->GetUserPath(user);
    if (!QDir(path).removeRecursively())
        throw new Exception("Unable to remove: " + path, BOOST_CURRENT_FUNCTION);

    this->refreshUserDB();
}

void DatabaseBin::UnlockUser(User *user)
{
    Q_UNUSED(user);
    this->refreshUserDB();
}

void DatabaseBin::LockUser(User *user)
{
    Q_UNUSED(user);
    this->refreshUserDB();
}

void DatabaseBin::LoadWindows()
{

}

void DatabaseBin::LoadText()
{

}

void DatabaseBin::StoreScrollback(User *owner, Scrollback *sx)
{

}

void DatabaseBin::StoreNetwork(IRCSession *session)
{

}

void DatabaseBin::StoreUser(User *item)
{
    this->writeUser(item);
}

void DatabaseBin::UpdateUser(User *user)
{
    this->writeUser(user);
}

QHash<QString, QByteArray> DatabaseBin::GetStorage(user_id_t user)
{
    QHash<QString, QByteArray> results;

    return results;
}

void DatabaseBin::InsertStorage(user_id_t user, QString key, QByteArray data)
{

}

void DatabaseBin::UpdateStorage(user_id_t user, QString key, QByteArray data)
{

}

void DatabaseBin::RemoveStorage(user_id_t user, QString key)
{

}

void DatabaseBin::writeUser(User *us)
{
    QString path = this->GetUserConfigPath(us);
    if (!QDir(path).exists() && !QDir().mkpath(path))
        throw new Exception("Unable to create path: " + path, BOOST_CURRENT_FUNCTION);

    this->refreshUserDB();
}

void DatabaseBin::refreshUserDB()
{
    QString path = this->GetGlobalConfigPath();
    if (!QDir(path).exists() && !QDir().mkpath(path))
        throw new Exception("Unable to create path: " + path, BOOST_CURRENT_FUNCTION);

    // Serialize all data
    QHash<QString, QVariant> user_list;
    foreach (User *user, User::UserInfo)
    {
        QHash<QString, QVariant> user_info;
        user_info.insert("name", user->GetName());
        user_info.insert("pw", user->GetPassword());
        user_info.insert("id", user->GetID());
        user_info.insert("role", user->GetRole()->GetName());
        user_info.insert("is_locked", user->IsLocked());

        user_list.insert(QString::number(user->GetID()), user_info);
    }

    this->writeDataFile(path + "users.dat", ToArray(user_list));
}

bool DatabaseBin::writeDataFile(QString path, QByteArray data)
{
    QFile file(path);
    if (!file.open(QIODevice::Truncate | QIODevice::ReadWrite))
        throw new Exception("Unable to open DB for read / write: " + path, BOOST_CURRENT_FUNCTION);

    // First write magic code so that we can check compatibility with other versions
    file.write(ToArray(this->magic.size()));
    file.write(this->magic);

    // Write data
    file.write(ToArray(data.size()));
    file.write(data);

    file.close();
    return true;
}

QHash<QString, QVariant> DatabaseBin::loadSingleQHash(QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        throw new Exception("Unable to open DB for read: " + path, BOOST_CURRENT_FUNCTION);
    QHash<QString, QVariant> results;

    // Get size of magic
    QByteArray size_of_magic = file.read(this->headerSize);
    int i_size_of_magic = intFromArray(&size_of_magic);

    // Get magic
    QByteArray b_magic = file.read(static_cast<qint64>(i_size_of_magic));
    QHash<QString, QVariant> h_magic = hashFromArray(&b_magic);
    if (!h_magic.contains("grumpy_version"))
        throw new Exception("Invalid DB header (corrupted data?): " + path, BOOST_CURRENT_FUNCTION);

    QByteArray b_size_of_hash = file.read(this->headerSize);
    int i_size_of_hash = intFromArray(&b_size_of_hash);

    QByteArray b_hash = file.read(static_cast<qint64>(i_size_of_hash));
    results = hashFromArray(&b_hash);
    return results;
}

bool DatabaseBin::isLegal(QString name)
{
    return name.contains(QRegExp( "[" + QRegExp::escape( "\\/:*?\"<>|" ) + "]" ));
}

QString DatabaseBin::normalizePath(QString path)
{
    return path.replace(QRegExp( "[" + QRegExp::escape( "\\/:*?\"<>|" ) + "]" ), QString( "_" ));
}

QString DatabaseBin::GetRootPath()
{
    return this->PathPrefix + QDir().separator();
}

QString DatabaseBin::GetGlobalConfigPath()
{
    return this->GetRootPath() + "config" + QDir().separator();
}

QString DatabaseBin::GetUserPath(User *user)
{
    return this->GetRootPath() + "users" + QDir().separator() + user->GetName() + QDir().separator();
}

QString DatabaseBin::GetUserConfigPath(User *user)
{
    return this->GetUserPath(user) + "configuration" + QDir().separator();
}

QString DatabaseBin::GetScrollbackPath(Scrollback *scrollback, User *user)
{
    QString path = "unknown";
    switch (scrollback->GetType())
    {
        case ScrollbackType_Channel:
            path = "channels";
            break;
        case ScrollbackType_System:
            path = "system";
            break;
        case ScrollbackType_User:
            path = "users";
            break;
    }
    return this->GetUserPath(user) + path + QDir().separator() + QString::number(scrollback->GetID()) + this->Suffix;
}
