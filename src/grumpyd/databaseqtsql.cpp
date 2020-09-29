//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2020

#include "databaseqtsql.h"
#include "grumpyconf.h"
#include "virtualscrollback.h"
#include "../libcore/core.h"
#include "../libcore/generic.h"
#include "../libcore/exception.h"
#include "../libcore/eventhandler.h"
#include "session.h"
#include "user.h"
#include "security.h"
#include "syncableircsession.h"
#include <QtSql>

using namespace GrumpyIRC;

static QString ListToString(QList<Scrollback*> list)
{
    QString result;
    foreach (Scrollback *scroll, list)
        result += QString::number(scroll->GetOriginalID()) + ",";

    return result;
}

static QList<scrollback_id_t> StringToList(const QString& list)
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

void DatabaseQtSQL::CheckDriver()
{
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
    {
        // This is only affecting work-in-progress Qt rewrite of this driver, so in fact doesn't do much now
        // generic sqlite driver will still work when this is false
        CONF->SQLite_Enabled = false;
        GRUMPY_LOG("WARNING: SqlLite driver is not available, support for sqlite disabled");
    }
    if (!QSqlDatabase::drivers().contains("QPSQL"))
    {
        CONF->PSQL_Enabled = false;
        GRUMPY_LOG("WARNING: psql driver is not available, support for PostgreSQL disabled");
    }
}

void DatabaseQtSQL::LoadRoles()
{
    Role::Defaults();
}

void DatabaseQtSQL::LoadUsers()
{
    Q_ASSERT(User::UserInfo.empty());
    QSqlQuery userlist = this->db.exec("SELECT id, name, password, role, is_locked FROM users ORDER BY id;");
    userlist.setForwardOnly(true);

    if (!userlist.isActive())
    {
        this->fail("DB corrupted, can't select from users table: " + userlist.lastError().text());
        return;
    }

    while (userlist.next())
    {
        User *ux = new User(userlist.value(1).toString(), userlist.value(2).toString(), userlist.value(0).toInt(), userlist.value(4).toBool());
        User::UserInfo.append(ux);
        if (Role::Roles.contains(userlist.value(3).toString()))
            ux->SetRole(Role::Roles[userlist.value(3).toString()]);
        ux->StorageLoad();
    }
}

void DatabaseQtSQL::LoadSessions()
{
    QSqlQuery networks = this->db.exec("SELECT id, network_id, user_id, hostname, port, ssl, nick, ident, system_id, password, scrollback_list, name FROM networks ORDER BY id;");

    if (!networks.isActive())
        throw new Exception("Unable to load networks from db: " + networks.lastError().text(), BOOST_CURRENT_FUNCTION);

    unsigned int lnid = 0;
    while (networks.next())
    {
        unsigned int nid = networks.value(1).toUInt();
        User *user = User::GetUser(networks.value(2).toUInt());
        if (!user)
        {
            if (!CONF->AutoFix)
            {
                GRUMPY_ERROR("Missing owner for a network, skipping initialization of scrollback, please run grumpyd with --cleanup parameter to fix this issue permanently");
            } else
            {
                GRUMPY_LOG("Removing network with no owner: " + QString::number(networks.value(0).toInt()));
                QSqlQuery q2 = this->db.exec("DELETE FROM networks WHERE id = " + QString::number(networks.value(0).toInt()) + ";");
                if (!q2.isActive())
                    throw new Exception("Unable to remove network: " + q2.lastError().text(), BOOST_CURRENT_FUNCTION);
            }
            continue;
        }
        Scrollback *system = user->GetScrollback(networks.value(8).toUInt());
        if (!system)
        {
            if (!CONF->AutoFix)
            {
                GRUMPY_ERROR("Missing system window for a network, skipping initialization, please run grumpyd with --cleanup parameter to fix this issue permanently");
            } else
            {
                GRUMPY_LOG("Removing network with no system window: " + QString::number(networks.value(0).toInt()));
                QSqlQuery q2 = this->db.exec("DELETE FROM networks WHERE id = " + QString::number(networks.value(0).toInt()) + ";");
                if (!q2.isActive())
                    throw new Exception("Unable to remove network: " + q2.lastError().text(), BOOST_CURRENT_FUNCTION);
            }
            continue;
        }
        QList<Scrollback*> scrollback_list;
        QList<scrollback_id_t> scrollback_temp = StringToList(networks.value(10).toString());
        system->SetDead(true);
        SyncableIRCSession *session = nullptr;
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
                    GRUMPY_LOG("Removing network with no window: " + QString::number(networks.value(0).toInt()));
                    QSqlQuery q2 = this->db.exec("DELETE FROM networks WHERE id = " + QString::number(networks.value(0).toInt()) + ";");
                    if (!q2.isActive())
                        throw new Exception("Unable to remove network: " + q2.lastError().text(), BOOST_CURRENT_FUNCTION);
                }
                goto next;
            }
            scrollback->SetDead(true);
            scrollback_list.append(scrollback);
        }

        session = new SyncableIRCSession(nid, system, user, scrollback_list);
        session->SetHostname(networks.value(3).toString());
        session->SetIdent(networks.value(7).toString());
        session->SetNick(networks.value(6).toString());
        session->SetSSL(Generic::Int2Bool(networks.value(5).toInt()));
        session->SetPort(networks.value(4).toUInt());
        session->SetName(networks.value(11).toString());
        if (lnid < nid)
            lnid = nid;
        user->RegisterSession(session);

        next:
            continue;
    }

    SyncableIRCSession::SetLastNID(lnid);
}

void DatabaseQtSQL::LoadWindows()
{
    scrollback_id_t max_id = 0;
    QSqlQuery windows = this->db.exec("SELECT original_id, user_id, target, type, virtual_state, last_item, parent, id, is_hidden FROM scrollbacks ORDER BY id;");
    windows.setForwardOnly(true);

    if (!windows.isActive())
        throw new Exception("Unable to recover scrollbacks from sql: " + windows.lastError().text(), BOOST_CURRENT_FUNCTION);

    // Let's instantiate all loaded scrollbacks, this is actually pretty dangerous, we need to make sure that they are properly registered
    // so that we prevent some horrid memory leak

    QHash<scrollback_id_t, Scrollback*> scrollbacks;
    while (windows.next())
    {
        //! \todo We assume that parent scrollbacks were registered BEFORE the scrollbacks that were inherited
        //! from them, but that doesn't need to be true and could cause troubles
        scrollback_id_t scrollback_id = windows.value(0).toUInt();
        scrollback_id_t parent_id = 0;
        if (!windows.value(6).isNull())
            parent_id = windows.value(6).toUInt();
        Scrollback *parent_ptr = nullptr;
        User *user = User::GetUser(windows.value(1).toUInt());
        if (!user)
        {
            if (!CONF->AutoFix)
            {
                GRUMPY_ERROR("Missing owner for a scrollback, skipping initialization of scrollback, please run grumpyd with --cleanup parameter to fix this issue permanently");
            } else
            {
                GRUMPY_LOG("Removing scrollback with no owner: " + QString::number(scrollback_id));
                this->ClearScrollback(scrollback_id, windows.value(1).toUInt());
                this->RemoveScrollback(windows.value(7).toUInt());
            }
            continue;
        }
        if (parent_id && scrollbacks.contains(parent_id))
            parent_ptr = scrollbacks[parent_id];
        QString target = windows.value(2).toString();
        if (max_id < scrollback_id)
            max_id = scrollback_id + 1;
        ScrollbackType st = static_cast<ScrollbackType>(windows.value(3).toInt());
        VirtualScrollback *sx = (VirtualScrollback*)Core::GrumpyCore->NewScrollback(parent_ptr, target, st);
        if (Generic::Int2Bool(windows.value(8).toInt()))
            sx->Hide();
        sx->SetOriginalID(scrollback_id);
        sx->SetLastItemID(windows.value(5).toInt());
        sx->SetOwner(user, true);
        if (!sx->PropertyBag.contains("initialized"))
            sx->PropertyBag.insert("initialized", QVariant(true));
        if (scrollbacks.contains(scrollback_id))
            throw new Exception("Multiple scrollbacks with same ID", BOOST_CURRENT_FUNCTION);
        scrollbacks.insert(scrollback_id, sx);
    }
    VirtualScrollback::SetLastID(++max_id);
}

void DatabaseQtSQL::LoadText()
{
    foreach (User *user, User::UserInfo)
    {
        foreach (VirtualScrollback *scrollback, user->GetScrollbacks())
        {
            QList<QVariant> input;
            input << QVariant(user->GetID())
                  << QVariant(scrollback->GetOriginalID());
            scrollback_id_t last_item = 0;
            QSqlQuery text = QSqlQuery(this->db);
            // FIXME - performance
            // We are getting last N items of scrollback, upside down and then we have to reinsert them (so we fetch them from last row to first one)
            // which is not natural for most SQL engines, it requires to store whole result in RAM and then process it
            // this logic may need some upgrade
            text.setForwardOnly(false);
            text.prepare("SELECT id, item_id, user_id, scrollback_id, date, type, nick, ident, host, text, self "\
                         "FROM scrollback_items "\
                         "WHERE user_id = :user_id AND scrollback_id = :scrollback_id "\
                         "ORDER BY item_id DESC LIMIT " + QString::number(CONF->GetMaxScrollbackSize()) + ";");

            text.bindValue(":user_id", user->GetID());
            text.bindValue(":scrollback_id", scrollback->GetOriginalID());

            if (!text.exec())
                throw new Exception("Unable to fetch: " + text.lastError().text(), BOOST_CURRENT_FUNCTION);

            // this must be long because we need something bigger than int as we convert unsigned int to signed variable
            long item = static_cast<long>(text.size());
            if (item > 0)
                item--;
            while (item >= 0)
            {
                if (!text.seek((unsigned int)item--))
                    throw new Exception("Unable to seek: " + text.lastError().text(), BOOST_CURRENT_FUNCTION);

                last_item = text.value(1).toUInt();
                QDateTime date = QDateTime::fromMSecsSinceEpoch(text.value(4).toLongLong());
                QString item_text = text.value(9).toString();
                ScrollbackItemType type = static_cast<ScrollbackItemType>(text.value(5).toInt());
                bool self = Generic::Int2Bool(text.value(10).toInt());
                libircclient::User user;
                user.SetNick(text.value(6).toString());
                user.SetIdent(text.value(7).toString());
                user.SetHost(text.value(8).toString());
                ScrollbackItem tm(item_text, type, user, date, last_item, self);
                scrollback->ImportText(tm);
            }

            scrollback->SetLastItemID(last_item);
        }
    }
}

void DatabaseQtSQL::Maintenance()
{
    if (CONF->DBTrim)
    {
        // Calculate ts
        QDateTime ts = QDateTime::currentDateTime().addDays(-10);
        int min_rows = 200;
        GRUMPY_LOG("Removing items from table scrollback_items that are older than 10 days, if there is more than " + QString::number(min_rows) + " items in window");
        // Now, fetch all scrollbacks that have at least 200 items
        this->LoadUsers();
        this->LoadWindows();
        foreach (Scrollback *scrollback, Scrollback::ScrollbackList)
        {
            QDateTime start_time = QDateTime::currentDateTime();

            // Figure out how many items we have there
            QSqlQuery sql_c(this->db);
            sql_c.prepare("SELECT count(1) FROM scrollback_items WHERE scrollback_id = :id;");
            sql_c.bindValue(":id", scrollback->GetOriginalID());

            if (!sql_c.exec())
            {
                GRUMPY_ERROR("Unable to fetch count of scrollback " + QString::number(scrollback->GetOriginalID()) + " <" + scrollback->GetTarget() + ">: " + sql_c.lastError().text());
                return;
            }
            sql_c.first();
            int count = sql_c.value(0).toInt();
            if (count > min_rows)
            {
                // Get id of item last_item - min_rows so that we know what we don't want to delete
                QSqlQuery scrollback_last_item(this->db);
                scrollback_last_item.prepare("SELECT id FROM scrollback_items WHERE scrollback_id = :id ORDER BY id DESC LIMIT " +
                                                                                                    QString::number(min_rows));
                scrollback_last_item.bindValue(":id", scrollback->GetOriginalID());

                if (!scrollback_last_item.exec())
                {
                    GRUMPY_ERROR("Unable to fetch last item id of scrollback " + QString::number(scrollback->GetOriginalID()) + " <" + scrollback->GetTarget() + ">: " + scrollback_last_item.lastError().text());
                    return;
                }

                if (scrollback_last_item.size() < min_rows)
                {
                    GRUMPY_ERROR("Unable to fetch last item id of scrollback " + QString::number(scrollback->GetOriginalID()) + " <" + scrollback->GetTarget() + ">: too few rows");
                    return;
                }

                if (!scrollback_last_item.seek(min_rows - 1))
                {
                    GRUMPY_ERROR("Can't seek: " + scrollback_last_item.lastError().text());
                    return;
                }

                int last_id = scrollback_last_item.value(0).toInt();

                GRUMPY_LOG("Trimming items of " + QString::number(scrollback->GetOriginalID()) + " <" + scrollback->GetTarget() + "> which has " + QString::number(count) +
                           " items (last id: " + QString::number(last_id) + ")");
                if (!this->ExecuteNonQuery("DELETE FROM scrollback_items WHERE scrollback_id = " + QString::number(scrollback->GetOriginalID()) + " AND id < " + QString::number(last_id)
                                                     + " AND date < " + QString::number(ts.toMSecsSinceEpoch()) + ";"))
                {
                    GRUMPY_ERROR("Failed to delete items: " + this->db.lastError().text());
                    return;
                }
                int changed = this->lastNumRowsAffected;
                GRUMPY_LOG("OK: removed " + QString::number(changed) + " records in " + QString::number(start_time.secsTo(QDateTime::currentDateTime())) + "s" +
                           " remaining item count: " + QString::number(count - changed));
            } else
            {
                GRUMPY_LOG("Skipping cleanup of scrollback " + QString::number(scrollback->GetOriginalID()) + " <" + scrollback->GetTarget() + "> because it has only " + QString::number(count) + " items");
            }
        }
        GRUMPY_LOG("Performing database optimization");
        if (!this->ExecuteNonQuery("VACUUM ANALYZE;"))
        {
            GRUMPY_ERROR("Failed to vacuum: " + this->db.lastError().text());
            return;
        }
    }
}

void DatabaseQtSQL::StoreUser(User *item)
{
    QSqlQuery query(this->db);
    query.prepare("INSERT INTO users (id, name, password, is_locked, role) VALUES (:id, :name, :password, :locked, :role)");
    query.bindValue(":id", item->GetID());
    query.bindValue(":name", item->GetName());
    query.bindValue(":password", item->GetPassword());
    query.bindValue(":locked", item->IsLocked());
    query.bindValue(":role", item->GetRole()->GetName());

    if (!query.exec())
        throw new Exception("Unable to store user record: " + query.lastError().text(), BOOST_CURRENT_FUNCTION);
}

void DatabaseQtSQL::StoreNetwork(IRCSession *session)
{

}

QList<QVariant> DatabaseQtSQL::FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size)
{
    return scrollback->OriginFetchBacklog(from, size);
}

QList<QVariant> DatabaseQtSQL::Search(QString text, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

QList<QVariant> DatabaseQtSQL::SearchRegular(QString regex, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

QList<QVariant> DatabaseQtSQL::SearchOne(VirtualScrollback *scrollback, QString text, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

QList<QVariant> DatabaseQtSQL::SearchOneRegular(VirtualScrollback *scrollback, QString regex, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

void DatabaseQtSQL::UpdateUser(User *user)
{

}

void DatabaseQtSQL::RemoveNetwork(IRCSession *session)
{

}

void DatabaseQtSQL::RemoveUser(User *user)
{

}

void DatabaseQtSQL::RemoveScrollback(unsigned int id)
{

}

void DatabaseQtSQL::RemoveScrollback(User *owner, Scrollback *sx)
{

}

void DatabaseQtSQL::LockUser(User *user)
{

}

void DatabaseQtSQL::UnlockUser(User *user)
{

}

void DatabaseQtSQL::ClearScrollback(User *owner, Scrollback *sx)
{

}

void DatabaseQtSQL::ClearScrollback(unsigned int id, unsigned int user_id)
{

}

void DatabaseQtSQL::StoreScrollback(User *owner, Scrollback *sx)
{

}

void DatabaseQtSQL::UpdateNetwork(IRCSession *session)
{

}

void DatabaseQtSQL::StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item)
{

}

void DatabaseQtSQL::UpdateRoles()
{

}

QHash<QString, QVariant> DatabaseQtSQL::GetConfiguration(user_id_t user)
{
    QHash<QString, QVariant> hash;
    return hash;
}

void DatabaseQtSQL::SetConfiguration(user_id_t user, QHash<QString, QVariant> data)
{

}

QHash<QString, QByteArray> DatabaseQtSQL::GetStorage(user_id_t user)
{
    return QHash<QString, QByteArray>();
}

void DatabaseQtSQL::InsertStorage(user_id_t user, QString key, QByteArray data)
{

}

void DatabaseQtSQL::UpdateStorage(user_id_t user, QString key, QByteArray data)
{

}

void DatabaseQtSQL::RemoveStorage(user_id_t user, QString key)
{

}

bool DatabaseQtSQL::ExecuteNonQuery(QString sql)
{
    QSqlQuery x = this->db.exec(sql);
    this->lastNumRowsAffected = x.numRowsAffected();
    return x.isActive();
}

bool DatabaseQtSQL::ExecuteFile(QString file_src, QString *error)
{
    QList<QString> statements = file_src.split(";");
    foreach (QString sx, statements)
    {
        // Qt perform replace on current string instance, instead of copy, so we need to explicitly copy it to test it
        QString sx_copy = sx;
        if (sx_copy.replace("\n", "").replace(" ", "").isEmpty())
            continue;
        QSqlQuery q(this->db);
        if (!q.exec(sx))
        {
            *error = q.lastError().text() + " (query: " + sx + ")";
            return false;
        }
    }
    return true;
}

QString DatabaseQtSQL::GetSource(QString name)
{
    QFile file(":/sql/" + name);
    if (!file.open(QIODevice::ReadOnly))
        throw new Exception("Unable to open internal resource: " + name, BOOST_CURRENT_FUNCTION);

    return QString(file.readAll());
}

bool DatabaseQtSQL::IsFailed()
{
    return this->isFailed;
}

QString DatabaseQtSQL::GetLastErrorText()
{
    return this->failureReason;
}

void DatabaseQtSQL::fail(QString reason)
{
    GRUMPY_ERROR("PSQL driver failed: " + reason);
    this->isFailed = true;
    this->failureReason = reason;
}

