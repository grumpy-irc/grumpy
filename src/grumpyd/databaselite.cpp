//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifdef GRUMPY_SQLITE

#include "virtualscrollback.h"
#include "../libcore/generic.h"
#include "../libcore/exception.h"
#include "grumpyconf.h"
#include "grumpyd.h"
#include "../libcore/eventhandler.h"
#include "session.h"
#include "../libcore/core.h"
#include "user.h"
#include "security.h"
#include "databaselite.h"
#include <string>
#include <QFile>
#include <QStringList>
#include "syncableircsession.h"

using namespace GrumpyIRC;

static QString ListToString(QList<Scrollback*> list)
{
    QString result;
    foreach (Scrollback *scroll, list)
        result += QString::number(scroll->GetOriginalID()) + ",";

    return result;
}

static QList<scrollback_id_t> StringToList(QString list)
{
    QList<scrollback_id_t> rx;
    QList<QString> temp = list.split(',');
    foreach (QString t2, temp)
    {
        if (t2.isEmpty())
            continue;
        rx.append(t2.toUInt());
    }
    return rx;
}

QString DatabaseLite::GetSource(QString name)
{
    QFile file(":/sql/" + name);
    if (!file.open(QIODevice::ReadOnly))
        throw new Exception("Unable to open internal resource: " + name, BOOST_CURRENT_FUNCTION);

    return QString(file.readAll());
}

DatabaseLite::DatabaseLite()
{
    QString datafile = Grumpyd::GetCFPath() + "sqlite.dat";
    bool install = !QFile().exists(datafile);
    this->database = new SQLite(datafile);
    this->last_user_id = 0;
    if (install)
    {
        if (!this->database->ExecuteNonQuery(GetSource("install.sql")))
            throw new Exception("Failed to initialize SQL storage: " + this->LastError, BOOST_CURRENT_FUNCTION);

        // Let's set grumpy to init mode
        CONF->Init = true;
    }
}

DatabaseLite::~DatabaseLite()
{
    delete this->database;
    this->database = NULL;
}

void DatabaseLite::LoadRoles()
{
    Role::Defaults();
}

void DatabaseLite::LoadUsers()
{
    SqlResult *userlist = this->database->ExecuteQuery("SELECT id, name, password, role FROM users;");
    if (!userlist->Count())
    {
        CONF->Init = true;
    } else
    {
        unsigned int user = 0;
        while (user < userlist->Count())
        {
            SqlRow row = userlist->GetRow(user++);
            User *ux = new User(row.GetField(1).toString(), row.GetField(2).toString(), row.GetField(0).toInt());
            User::UserInfo.append(ux);
            if (Role::Roles.contains(row.GetField(3).toString()))
                ux->SetRole(Role::Roles[row.GetField(3).toString()]);
        }
    }
    delete userlist;
}

void DatabaseLite::LoadSessions()
{
    SqlResult *networks = this->database->ExecuteQuery("SELECT id, network_id, user_id, hostname, port, ssl, nick, ident, system_id, password, scrollback_list, name FROM networks;");

    if (networks->InError)
        throw new Exception("Unable to load networks from db: " + this->LastError, BOOST_CURRENT_FUNCTION);

    unsigned int network = 0;
    unsigned int lnid = 0;
    while (network < networks->Count())
    {
        SqlRow row = networks->GetRow(network++);
        unsigned int nid = row.GetField(1).toUInt();
        User *user = User::GetUser(row.GetField(2).toUInt());
        if (!user)
        {
            if (!CONF->AutoFix)
            {
                GRUMPY_ERROR("Missing owner for a network, skipping initialization of scrollback, please run grumpyd with --cleanup parameter to fix this issue permanently");
            } else
            {
                GRUMPY_LOG("Removing network with no owner");
                this->database->ExecuteNonQuery("DELETE FROM networks WHERE id = " + QString::number(row.GetField(0).toInt()) + ";");
            }
            continue;
        }
        Scrollback *system = user->GetScrollback(row.GetField(8).toUInt());
        if (!system)
        {
            if (!CONF->AutoFix)
            {
                GRUMPY_ERROR("Missing system window for a network, skipping initialization, please run grumpyd with --cleanup parameter to fix this issue permanently");
            } else
            {
                GRUMPY_LOG("Removing network with no owner");
                this->database->ExecuteNonQuery("DELETE FROM networks WHERE id = " + QString::number(row.GetField(0).toInt()) + ";");
            }
            continue;
        }
        QList<Scrollback*> scrollback_list;
        QList<scrollback_id_t> scrollback_temp = StringToList(row.GetField(10).toString());
        system->SetDead(true);
        SyncableIRCSession *session = NULL;
        foreach (scrollback_id_t item, scrollback_temp)
        {
            Scrollback *scrollback = user->GetScrollback(item);
            if (!scrollback)
            {
                if (!CONF->AutoFix)
                {
                    GRUMPY_ERROR("Missing window for a network, skipping initialization, please run grumpyd with --cleanup parameter to fix this issue permanently");
                } else
                {
                    GRUMPY_LOG("Removing network with no window");
                    this->database->ExecuteNonQuery("DELETE FROM networks WHERE id = " + QString::number(row.GetField(0).toInt()) + ";");
                }
                goto next;
            }
            scrollback->SetDead(true);
            scrollback_list.append(scrollback);
        }

        session = new SyncableIRCSession(nid, system, user, scrollback_list);
        session->SetHostname(row.GetField(3).toString());
        session->SetIdent(row.GetField(7).toString());
        session->SetNick(row.GetField(6).toString());
        session->SetSSL(Generic::Int2Bool(row.GetField(5).toInt()));
        session->SetPort(row.GetField(4).toUInt());
        session->SetName(row.GetField(11).toString());
        if (lnid < nid)
            lnid = nid;
        user->RegisterSession(session);

        next:
            continue;
    }

    SyncableIRCSession::SetLastNID(lnid);

    delete networks;
}

void DatabaseLite::LoadWindows()
{
    scrollback_id_t max_id = 0;
    SqlResult *windows = this->database->ExecuteQuery("SELECT original_id, user_id, target, type, virtual_state, last_item, parent, id FROM scrollbacks;");
    if (windows->InError)
        throw new Exception("Unable to recover scrollbacks from sql: " + this->LastError, BOOST_CURRENT_FUNCTION);

    // Let's instantiate all loaded scrollbacks, this is actually pretty dangerous, we need to make sure that they are properly registered
    // so that we prevent some horrid memory leak

    unsigned int window = 0;
    QHash<scrollback_id_t, Scrollback*> scrollbacks;
    while (window < windows->Count())
    {
        SqlRow row = windows->GetRow(window++);
        //! \todo We assume that parent scrollbacks were registered BEFORE the scrollbacks that were inherited
        //! from them, but that doesn't need to be true and could cause troubles
        scrollback_id_t scrollback_id = row.GetField(0).toUInt();
        scrollback_id_t parent_id = 0;
        if (!row.GetField(6).isNull())
            parent_id = row.GetField(6).toUInt();
        Scrollback *parent_ptr = NULL;
        User *user = User::GetUser(row.GetField(1).toUInt());
        if (!user)
        {
            if (!CONF->AutoFix)
            {
                GRUMPY_ERROR("Missing owner for a scrollback, skipping initialization of scrollback, please run grumpyd with --cleanup parameter to fix this issue permanently");
            } else
            {
                GRUMPY_LOG("Removing scrollback with no owner");
                this->ClearScrollback(scrollback_id, row.GetField(1).toUInt());
                this->RemoveScrollback(row.GetField(7).toUInt());
            }
            continue;
        }
        if (parent_id && scrollbacks.contains(parent_id))
            parent_ptr = scrollbacks[parent_id];
        QString target = row.GetField(2).toString();
        if (max_id < scrollback_id)
            max_id = scrollback_id + 1;
        ScrollbackType st = static_cast<ScrollbackType>(row.GetField(3).toInt());
        VirtualScrollback *sx = (VirtualScrollback*)Core::GrumpyCore->NewScrollback(parent_ptr, target, st);
        sx->SetOriginalID(scrollback_id);
        sx->SetLastItemID(row.GetField(5).toInt());
        sx->SetOwner(user, true);
        if (!sx->PropertyBag.contains("initialized"))
            sx->PropertyBag.insert("initialized", QVariant(true));
        if (scrollbacks.contains(scrollback_id))
            throw new Exception("Multiple scrollbacks with same ID", BOOST_CURRENT_FUNCTION);
        scrollbacks.insert(scrollback_id, sx);
    }
    VirtualScrollback::SetLastID(++max_id);

    delete windows;
}

void DatabaseLite::LoadText()
{
    foreach (User *user, User::UserInfo)
    {
        foreach (VirtualScrollback *scrollback, user->GetScrollbacks())
        {
            QList<QVariant> input;
            input << QVariant(user->GetID())
                  << QVariant(scrollback->GetOriginalID());
            scrollback_id_t last_item = 0;
            SqlResult *text = this->database->ExecuteQuery_Bind("SELECT id, item_id, user_id, scrollback_id, date, type, nick, ident, host, text, self "\
                                                                 "FROM scrollback_items "\
                                                                 "WHERE user_id = ? AND scrollback_id = ? "\
                                                                 "ORDER BY item_id ASC LIMIT " + QString::number(CONF->GetMaxScrollbackSize()) + ";", input);
            if (text->InError)
                throw new Exception("Unable to fetch: " + this->LastError, BOOST_CURRENT_FUNCTION);

            unsigned int item = 0;
            while (item < text->Count())
            {
                SqlRow row = text->GetRow(item++);
                last_item = row.GetField(1).toUInt();
                QString item_text = row.GetField(9).toString();
                ScrollbackItemType type = static_cast<ScrollbackItemType>(row.GetField(5).toInt());
                bool self = Generic::Int2Bool(row.GetField(10).toInt());
                libircclient::User user;
                user.SetNick(row.GetField(6).toString());
                user.SetIdent(row.GetField(7).toString());
                user.SetHost(row.GetField(8).toString());
                ScrollbackItem tm(item_text, type, user, last_item, self);
                scrollback->ImportText(tm);
            }

            scrollback->SetLastItemID(last_item);

            delete text;
        }
    }
}

QHash<QString, QVariant> DatabaseLite::GetConfiguration(user_id_t user)
{
    QHash<QString, QVariant> results;

    return results;
}

int DatabaseLite::GetLastUserID()
{
    return this->last_user_id;
}

void DatabaseLite::SetConfiguration(user_id_t user, QHash<QString, QVariant> data)
{

}

void DatabaseLite::StoreScrollback(User *owner, Scrollback *sx)
{
    QString sql = "INSERT INTO scrollbacks (original_id, user_id, target, type, virtual_state, last_item) VALUES (?,?,?,?,?,?);";
    Scrollback *parent = sx->GetParentScrollback();
    if (parent)
        sql = "INSERT INTO scrollbacks (original_id, user_id, target, type, virtual_state, last_item, parent) VALUES (?,?,?,?,?,?,?);";
    SqlResult *result;
    QStringList params;
    params << QString::number(sx->GetOriginalID())
           << QString::number(owner->GetID())
           << sx->GetTarget()
           << QString::number(static_cast<int>(sx->GetType()))
           << QString::number(static_cast<int>(sx->GetState()))
           << QString::number(sx->GetLastID());
    if (parent)
        params << QString::number(parent->GetOriginalID());
    result = this->database->ExecuteQuery_Bind(sql, params);
    if (result->InError)
    {
        delete result;
        throw new Exception("Unable to insert scrollback to db: " + this->LastError, BOOST_CURRENT_FUNCTION);
    }

    delete result;
}

void DatabaseLite::StoreUser(User *item)
{
    QStringList Parameters;
    Parameters << QString::number(item->GetID())
               << item->GetName()
               << item->GetPassword();
    QString SQL;

    if (item->GetRole())
    {
        SQL = "INSERT INTO users (id, name, password, is_locked, role) VALUES (?,?,?,0,?);";
        Parameters << item->GetRole()->GetName();
    } else
    {
        SQL = "INSERT INTO users (id, name, password, is_locked) VALUES (?,?,?,0);";
    }

    SqlResult *result = this->database->ExecuteQuery_Bind(SQL, Parameters);
    if (result->InError)
    {
        delete result;
        throw new Exception("Unable to store user record: " + this->LastError, BOOST_CURRENT_FUNCTION);
    }

    delete result;
}

void DatabaseLite::UpdateUser(User *user)
{

}

void DatabaseLite::ClearScrollback(User *owner, Scrollback *sx)
{
    QString sql = "DELETE FROM scrollback_items WHERE scrollback_id = ? AND user_id = ?;";
    SqlResult *result;
    QStringList params;

    params << QString::number(sx->GetOriginalID())
           << QString::number(owner->GetID());

    result = this->database->ExecuteQuery_Bind(sql, params);

    if (result->InError)
    {
        delete result;
        throw new Exception("Unable to remove items from db: " + this->LastError, BOOST_CURRENT_FUNCTION);
    }

    delete result;
}

void DatabaseLite::ClearScrollback(unsigned int id, unsigned int user_id)
{
    QString sql = "DELETE FROM scrollback_items WHERE scrollback_id = ? AND user_id = ?;";
    SqlResult *result;
    QStringList params;

    params << QString::number(id)
           << QString::number(user_id);

    result = this->database->ExecuteQuery_Bind(sql, params);

    if (result->InError)
    {
        delete result;
        throw new Exception("Unable to remove items from db: " + this->LastError, BOOST_CURRENT_FUNCTION);
    }

    delete result;
}

void DatabaseLite::RemoveNetwork(IRCSession *session)
{
    QString sql = "DELETE FROM networks WHERE user_id = ? AND network_id = ?;";
    SqlResult *result;
    QList<QVariant> params;

    params << ((SyncableIRCSession*)session)->GetOwner()->GetID();
    params << session->GetSID();

    result = this->database->ExecuteQuery_Bind(sql, params);

    if (result->InError)
    {
        delete result;
        throw new Exception("Unable to remove network using sql: " + this->LastError, BOOST_CURRENT_FUNCTION);
    }

    delete result;
}

void DatabaseLite::RemoveScrollback(unsigned int id)
{
    QString sql = "DELETE FROM scrollbacks WHERE id = ?;";
    SqlResult *result;
    QStringList params;

    params << QString::number(id);

    result = this->database->ExecuteQuery_Bind(sql, params);

    if (result->InError)
    {
        delete result;
        throw new Exception("Unable to remove scrollback using sql: " + this->LastError, BOOST_CURRENT_FUNCTION);
    }

    delete result;
}

QList<QVariant> DatabaseLite::FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size)
{
    QList<QVariant> result;

    if (from > scrollback->GetLastID())
        return result;

    if (size > from)
    {
        size = from;
    }

    scrollback_id_t first;
    if (((long)from - (long)size) < 0)
        first = 0;
    else
        first = from - size;

    QList<QVariant> input;
    input << QVariant(scrollback->GetOwner()->GetID())
          << QVariant(scrollback->GetOriginalID())
          << QVariant(from)
          << QVariant(first);
    scrollback_id_t last_item = 0;
    SqlResult *text = this->database->ExecuteQuery_Bind("SELECT id, item_id, user_id, scrollback_id, date, type, nick, ident, host, text, self "\
                                                        "FROM scrollback_items "\
                                                        "WHERE user_id = ? AND scrollback_id = ? AND item_id < ? AND item_id >= ? "\
                                                        "ORDER BY item_id ASC;", input);
    if (text->InError)
        throw new Exception("Unable to fetch: " + this->LastError, BOOST_CURRENT_FUNCTION);

    unsigned int item = 0;
    while (item < text->Count())
    {
        SqlRow row = text->GetRow(item++);
        last_item = row.GetField(1).toUInt();
        QString item_text = row.GetField(9).toString();
        ScrollbackItemType type = static_cast<ScrollbackItemType>(row.GetField(5).toInt());
        bool self = Generic::Int2Bool(row.GetField(10).toInt());
        libircclient::User user;
        user.SetNick(row.GetField(6).toString());
        user.SetIdent(row.GetField(7).toString());
        user.SetHost(row.GetField(8).toString());
        ScrollbackItem tm(item_text, type, user, last_item, self);
        result.append(tm.ToHash());
    }
    delete text;
    return result;
}

void DatabaseLite::UpdateNetwork(IRCSession *session)
{
    QString sql = "UPDATE networks SET scrollback_list = ?, nick = ? WHERE network_id = ? AND user_id = ?;";
    QList<QVariant> hash;
    hash << (QVariant(ListToString(session->GetScrollbacks())))
         << QVariant(session->GetNick())
         << QVariant(session->GetSID())
         << QVariant((((SyncableIRCSession*)session))->GetOwner()->GetID());

    SqlResult *qx = this->database->ExecuteQuery_Bind(sql, hash);
    if (qx->InError)
        throw new Exception("SQL: " + this->LastError, BOOST_CURRENT_FUNCTION);
    delete qx;
}

void DatabaseLite::RemoveScrollback(User *owner, Scrollback *sx)
{
    this->ClearScrollback(owner, sx);
    QString sql = "DELETE FROM scrollbacks WHERE original_id = ? AND user_id = ?;";
    SqlResult *result;
    QStringList params;

    params << QString::number(sx->GetOriginalID())
           << QString::number(owner->GetID());

    result = this->database->ExecuteQuery_Bind(sql, params);

    if (result->InError)
        throw new Exception("Unable to remove scrollback from db: " + this->LastError, BOOST_CURRENT_FUNCTION);

    delete result;
}

void DatabaseLite::UpdateRoles()
{

}

void DatabaseLite::StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item)
{
    SqlResult *result;
    QList<QVariant> info;

    info << QVariant(owner->GetID())
         << QVariant(scrollback->GetOriginalID())
         << QVariant(item->GetTime().toMSecsSinceEpoch())
         << QVariant(static_cast<int>(item->GetType()))
         << QVariant(item->GetUser().GetNick())
         << QVariant(item->GetUser().GetIdent())
         << QVariant(item->GetUser().GetHost())
         << QVariant(item->GetText())
         << QVariant(item->GetID())
         << QVariant(item->IsSelf());

    QString sql = "INSERT INTO scrollback_items (user_id, scrollback_id, date, type, nick, ident, host, text, item_id, self) VALUES (?,?,?,?,?,?,?,?,?,?);";
    result = this->database->ExecuteQuery_Bind(sql, info);
    if (result->InError)
        throw new Exception("Unable to insert data to db: " + this->LastError, BOOST_CURRENT_FUNCTION);

    delete result;
}

void DatabaseLite::StoreNetwork(IRCSession *session)
{
    SqlResult *result;
    QList<QVariant> info;

    SyncableIRCSession *sx = dynamic_cast<SyncableIRCSession*>(session);

    info << QVariant(sx->GetOwner()->GetID())
         << QVariant(sx->GetSID())
         << QVariant(sx->GetHostname())
         << QVariant(sx->GetPort())
         << QVariant(sx->UsingSSL())
         << QVariant(sx->GetNick())
         << QVariant(sx->GetIdent())
         << QVariant(sx->GetPassword())
         << QVariant(sx->GetSystemWindow()->GetOriginalID())
         << QVariant(ListToString(sx->GetScrollbacks()))
         << QVariant(sx->GetName());

    QString sql = "INSERT INTO networks (user_id, network_id, hostname, port, ssl, nick, ident, password, system_id, scrollback_list, autoreconnect, autoidentify, autorejoin, name) VALUES (?,?,?,?,?,?,?,?,?,?,0,0,0,?);";
    result = this->database->ExecuteQuery_Bind(sql, info);
    if (result->InError)
        throw new Exception("Unable to insert data to db: " + this->LastError, BOOST_CURRENT_FUNCTION);

    delete result;
}

#endif
