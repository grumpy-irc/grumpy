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
#include <QDataStream>
#include <QtSql>

using namespace GrumpyIRC;

static QByteArray ToArray(const QHash<QString, QVariant>& data)
{
    QByteArray result;
    QDataStream stream(&result, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_2);
    stream << data;
    return result;
}

static QHash<QString, QVariant> FromArray(QByteArray data)
{
    QDataStream stream(&data, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_2);
    QHash<QString, QVariant> rx;
    stream >> rx;
    return rx;
}

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

int DatabaseQtSQL::GetUserCount()
{
    QSqlQuery q = this->db.exec("SELECT COUNT(1) FROM users;");
    if (!q.isActive())
        throw new Exception("Failed to obtain users: " + this->db.lastError().text(), BOOST_CURRENT_FUNCTION);

    q.first();
    return q.value(0).toInt();
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
        unsigned int nid = static_cast<unsigned int>(networks.value(1).toInt());
        User *user = User::GetUser(networks.value(2).toInt());
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
        Scrollback *system = user->GetScrollback(networks.value(8).toInt());
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
        session->SetPort(networks.value(4).toInt());
        session->SetName(networks.value(11).toString());
        if (lnid < nid)
            lnid = nid;
        user->RegisterSession(session);

        next:
            continue;
    }

    SyncableIRCSession::SetLastNID(lnid);
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

                last_item = text.value(1).toInt();
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
        this->Maintenance_Specific();
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
    QSqlQuery query(this->db);
    SyncableIRCSession *sx = dynamic_cast<SyncableIRCSession*>(session);

    query.prepare("INSERT INTO networks (user_id, network_id, hostname, port, ssl, nick, ident, password, system_id, scrollback_list, autoreconnect, autoidentify, autorejoin, name) "\
                  "VALUES (:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,:14);");

    query.bindValue(":1", QVariant(sx->GetOwner()->GetID()));
    query.bindValue(":2", QVariant(sx->GetSID()));
    query.bindValue(":3", QVariant(sx->GetHostname()));
    query.bindValue(":4", QVariant(sx->GetPort()));
    query.bindValue(":5", QVariant(sx->UsingSSL()));
    query.bindValue(":6", QVariant(sx->GetNick()));
    query.bindValue(":7", QVariant(sx->GetIdent()));
    query.bindValue(":8", QVariant(sx->GetPassword()));
    query.bindValue(":9", QVariant(sx->GetSystemWindow()->GetOriginalID()));
    query.bindValue(":10", QVariant(ListToString(sx->GetScrollbacks())));
    query.bindValue(":11", false);
    query.bindValue(":12", false);
    query.bindValue(":13", false);
    query.bindValue(":14", QVariant(sx->GetName()));

    if (!query.exec())
        throw new Exception("Unable to insert data to db: " + query.lastError().text(), BOOST_CURRENT_FUNCTION);
}

QList<QVariant> DatabaseQtSQL::FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size)
{
    QDateTime t = QDateTime::currentDateTime();
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

    scrollback_id_t last_item = 0;

    QSqlQuery text(this->db);
    text.setForwardOnly(true);
    text.prepare("SELECT id, item_id, user_id, scrollback_id, date, type, nick, ident, host, text, self "\
                                                        "FROM scrollback_items "\
                                                        "WHERE user_id = :user_id AND scrollback_id = :scrollback_id AND item_id < :item_id_max AND item_id >= :item_id_min "\
                                                        "ORDER BY item_id ASC;");
    text.bindValue(":user_id", scrollback->GetOwner()->GetID());
    text.bindValue(":scrollback_id", scrollback->GetOriginalID());
    text.bindValue(":item_id_max", from);
    text.bindValue(":item_id_min", first);

    if (!text.exec())
        throw new Exception("Unable to fetch: " + text.lastError().text(), BOOST_CURRENT_FUNCTION);

    while (text.next())
    {
        last_item = text.value(1).toInt();
        QString item_text = text.value(9).toString();
        ScrollbackItemType type = static_cast<ScrollbackItemType>(text.value(5).toInt());
        QDateTime date = QDateTime::fromMSecsSinceEpoch(text.value(4).toLongLong());
        bool self = Generic::Int2Bool(text.value(10).toInt());
        libircclient::User user;
        user.SetNick(text.value(6).toString());
        user.SetIdent(text.value(7).toString());
        user.SetHost(text.value(8).toString());
        ScrollbackItem tm(item_text, type, user, date, last_item, self);
        result.append(tm.ToHash());
    }

    GRUMPY_DEBUG("Execution of FetchBacklog took " + QString::number(t.secsTo(QDateTime::currentDateTime())) + " seconds.", 1);

    return result;
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
    if (!this->ExecuteNonQuery("DELETE FROM networks WHERE user_id = " +
                               QString::number(((SyncableIRCSession*)session)->GetOwner()->GetID()) +
                               " AND network_id = " + QString::number(session->GetSID()) + ";"))
    {
        throw new Exception("Unable to remove network: " + this->db.lastError().text(), BOOST_CURRENT_FUNCTION);
    }
}

void DatabaseQtSQL::RemoveUser(User *user)
{
    user_id_t id = user->GetID();
    QList<QVariant> params;

    if (!this->db.transaction())
        goto error;

    if (!this->ExecuteNonQuery("DELETE FROM scrollback_items WHERE user_id = " + QString::number(id) + ";"))
        goto error;
    if (!this->ExecuteNonQuery("DELETE FROM networks WHERE user_id = " + QString::number(id) + ";"))
        goto error;
    if (!this->ExecuteNonQuery("DELETE FROM settings WHERE user_id = " + QString::number(id) + ";"))
        goto error;
    if (!this->ExecuteNonQuery("DELETE FROM users WHERE id = " + QString::number(id) + ";"))
        goto error;
    if (!this->ExecuteNonQuery("DELETE FROM scrollbacks WHERE user_id = " + QString::number(id) + ";"))
        goto error;
    if (!this->ExecuteNonQuery("DELETE FROM user_data WHERE user_id = " + QString::number(id) + ";"))
        goto error;


    if (!this->db.commit())
        goto error;

    return;

    error:
        GRUMPY_ERROR("SQL BUG: " + this->db.lastError().text());
        if (!this->db.rollback())
            GRUMPY_ERROR("SQL ROLLBACK FAIL: " + this->db.lastError().text());
        throw new Exception("Unable to remove user using sql: " + this->db.lastError().text(), BOOST_CURRENT_FUNCTION);
}

void DatabaseQtSQL::LockUser(User *user)
{
    user_id_t id = user->GetID();
    QSqlQuery q = this->db.exec("UPDATE users SET is_locked = true WHERE id = " + QString::number(id) + ";");
    if (!q.isActive())
        throw new Exception("Unable to lock user using sql: " + this->db.lastError().text(), BOOST_CURRENT_FUNCTION);
}

void DatabaseQtSQL::UnlockUser(User *user)
{
    user_id_t id = user->GetID();
    QSqlQuery q = this->db.exec("UPDATE users SET is_locked = false WHERE id = " + QString::number(id) + ";");
    if (!q.isActive())
        throw new Exception("Unable to unlock user using sql: " + this->db.lastError().text(), BOOST_CURRENT_FUNCTION);
}

void DatabaseQtSQL::ClearScrollback(User *owner, Scrollback *sx)
{
    this->OffloadSQL("DELETE FROM scrollback_items WHERE scrollback_id = " + QString::number(sx->GetOriginalID()) +
                     " AND user_id = " + QString::number(owner->GetID()) + ";", true);
}

void DatabaseQtSQL::ClearScrollback(unsigned int id, unsigned int user_id)
{
    this->OffloadSQL("DELETE FROM scrollback_items WHERE scrollback_id = " +
                               QString::number(id) +
                               " AND user_id = " + QString::number(user_id) +
                               ";", true);
}

void DatabaseQtSQL::LoadWindows()
{
    scrollback_id_t max_id = 0;
    QSqlQuery windows = this->db.exec("SELECT original_id, user_id, target, type, virtual_state, last_item, parent, id, is_hidden FROM scrollbacks ORDER BY original_id;");
    //windows.setForwardOnly(true);

    if (!windows.isActive())
        throw new Exception("Unable to recover scrollbacks from sql: " + windows.lastError().text(), BOOST_CURRENT_FUNCTION);

    // Let's instantiate all loaded scrollbacks, this is actually pretty dangerous, we need to make sure that they are properly registered
    // so that we prevent some horrid memory leak

    QHash<scrollback_id_t, Scrollback*> scrollbacks;
    while (windows.next())
    {
        //! \todo We assume that parent scrollbacks were registered BEFORE the scrollbacks that were inherited
        //! from them, but that doesn't need to be true and could cause troubles
        scrollback_id_t scrollback_id = static_cast<scrollback_id_t>(windows.value(0).toInt());
        scrollback_id_t parent_id = 0;
        if (!windows.value(6).isNull())
            parent_id = static_cast<scrollback_id_t>(windows.value(6).toInt());
        Scrollback *parent_ptr = nullptr;
        User *user = User::GetUser(windows.value(1).toInt());
        if (!user)
        {
            if (!CONF->AutoFix)
            {
                GRUMPY_ERROR("Missing owner for a scrollback, skipping initialization of scrollback, please run grumpyd with --cleanup parameter to fix this issue permanently");
            } else
            {
                GRUMPY_LOG("Removing scrollback with no owner: " + QString::number(scrollback_id));
                this->ClearScrollback(scrollback_id, windows.value(1).toInt());
                this->RemoveScrollback(windows.value(7).toInt());
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

void DatabaseQtSQL::StoreScrollback(User *owner, Scrollback *sx)
{
    QSqlQuery q(this->db);
    Scrollback *parent = sx->GetParentScrollback();
    if (parent)
        q.prepare("INSERT INTO scrollbacks (original_id, user_id, target, type, virtual_state, last_item, parent, is_hidden) VALUES (:1,:2,:3,:4,:5,:6,:8,:7);");
    else
        q.prepare("INSERT INTO scrollbacks (original_id, user_id, target, type, virtual_state, last_item, is_hidden) VALUES (:1,:2,:3,:4,:5,:6,:7);");
    q.bindValue(":1", sx->GetOriginalID());
    q.bindValue(":2", owner->GetID());
    q.bindValue(":3", sx->GetTarget());
    q.bindValue(":4", static_cast<int>(sx->GetType()));
    q.bindValue(":5", static_cast<int>(sx->GetState()));
    q.bindValue(":6", sx->GetLastID());
    q.bindValue(":7", sx->IsHidden());
    if (parent)
        q.bindValue(":8", parent->GetOriginalID());
    if (!q.exec())
    {
        throw new Exception("Unable to insert scrollback to db: " + q.lastError().text(), BOOST_CURRENT_FUNCTION);
    }
}

void DatabaseQtSQL::UpdateScrollback(User *owner, Scrollback *sx)
{
    QSqlQuery update_sb(this->db);
    if (!update_sb.prepare("UPDATE scrollbacks SET last_item = :last_item, is_hidden = :is_hidden, virtual_state = :virtual_state "\
                      "WHERE user_id = :user_id AND original_id = :original_id;"))
        throw new Exception("Unable to prepare update for scrollback: " + update_sb.lastError().text(), BOOST_CURRENT_FUNCTION);
    update_sb.bindValue(":last_item", sx->GetLastID());
    update_sb.bindValue(":user_id", owner->GetID());
    update_sb.bindValue(":original_id", sx->GetOriginalID());
    update_sb.bindValue(":is_hidden", sx->IsHidden());
    update_sb.bindValue(":virtual_state", static_cast<int>(sx->GetState()));
    if (!update_sb.exec())
        throw new Exception("Unable to update scrollback: " + update_sb.lastError().text(), BOOST_CURRENT_FUNCTION);
}

void DatabaseQtSQL::RemoveScrollback(unsigned int id)
{
    QSqlQuery q(this->db);
    if (!q.exec("DELETE FROM scrollbacks WHERE id = " + QString::number(id) + ";"))
        throw new Exception("Unable to remove scrollback from db: " + q.lastError().text(), BOOST_CURRENT_FUNCTION);
}

void DatabaseQtSQL::RemoveScrollback(User *owner, Scrollback *sx)
{
    this->ClearScrollback(owner, sx);
    QSqlQuery q(this->db);
    q.prepare("DELETE FROM scrollbacks WHERE original_id = :original_id AND user_id = :user_id;");
    q.bindValue(":original_id", sx->GetOriginalID());
    q.bindValue(":user_id", owner->GetID());
    if (!q.exec())
        throw new Exception("Unable to remove scrollback from db: " + q.lastError().text(), BOOST_CURRENT_FUNCTION);
}

void DatabaseQtSQL::UpdateNetwork(IRCSession *session)
{
    QSqlQuery sql(this->db);
    sql.prepare("UPDATE networks SET scrollback_list = :1, nick = :2 WHERE network_id = :3 AND user_id = :4;");
    sql.bindValue(":1", QVariant(ListToString(session->GetScrollbacks())));
    sql.bindValue(":2", QVariant(session->GetNick()));
    sql.bindValue(":3", QVariant(session->GetSID()));
    sql.bindValue(":4", QVariant((((SyncableIRCSession*)session))->GetOwner()->GetID()));
    if (!sql.exec())
        throw new Exception("SQL: " + sql.lastError().text(), BOOST_CURRENT_FUNCTION);
}

void DatabaseQtSQL::StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item)
{
    //! \todo Updating last_id is probably not necessary, whole column could be dropped as last ID is updated when
    //! items are being loaded anyway, updating last_id wasn't implemented for long time and barely caused
    //! visible problems
    // When we store new item, we should update last ID as well, but only in case this item is actually higher than last ID,
    // otherwise it's probably a bug or DB is already corrupted, so we just issue a warning without any real update
    bool update_last_id = true;
    if ((scrollback->GetLastID()-1) > item->GetID())
    {
        GRUMPY_DEBUG("Warning, scrollback " + QString::number(scrollback->GetID()) + " storing item_id that is LOWER than last_id of scrollback"\
                                                                                     " (" + QString::number(scrollback->GetLastID()-1) +
                                                                                     " > " + QString::number(item->GetID()) + ")", 1);
        update_last_id = false;
    }
    QSqlQuery q(this->db);
    q.prepare("INSERT INTO scrollback_items (user_id, scrollback_id, date, type, nick, ident, host, text, item_id, self) VALUES "\
              "(:1, :2, :3, :4, :5, :6, :7, :8, :9, :10);");
    q.bindValue(":1", QVariant(owner->GetID()));
    q.bindValue(":2", QVariant(scrollback->GetOriginalID()));
    q.bindValue(":3", QVariant(item->GetTime().toMSecsSinceEpoch()));
    q.bindValue(":4", QVariant(static_cast<int>(item->GetType())));
    q.bindValue(":5", QVariant(item->GetUser().GetNick()));
    q.bindValue(":6", QVariant(item->GetUser().GetIdent()));
    q.bindValue(":7", QVariant(item->GetUser().GetHost()));
    q.bindValue(":8", QVariant(item->GetText()));
    q.bindValue(":9", QVariant(item->GetID()));
    q.bindValue(":10", QVariant(item->IsSelf()));

    if (update_last_id)
        this->db.transaction();

    if (!q.exec())
    {
        if (update_last_id)
            this->db.rollback();
        throw new Exception("Unable to insert data to db: " + this->db.lastError().text(), BOOST_CURRENT_FUNCTION);
    }

    if (update_last_id)
    {
        QSqlQuery update_sb(this->db);
        update_sb.prepare("UPDATE scrollbacks SET last_item = :last_item WHERE user_id = :user_id AND original_id = :original_id;");
        update_sb.bindValue(":last_item", item->GetID());
        update_sb.bindValue(":user_id", owner->GetID());
        update_sb.bindValue(":original_id", scrollback->GetOriginalID());
        if (!update_sb.exec())
        {
            this->db.rollback();
            throw new Exception("Unable to update scrollback last_id: " + this->db.lastError().text(), BOOST_CURRENT_FUNCTION);
        }
    }
    if (update_last_id && !this->db.commit())
        throw new Exception("Failed to commit insertion of new item: " + this->db.lastError().text(), BOOST_CURRENT_FUNCTION);
}

void DatabaseQtSQL::UpdateRoles()
{

}

QHash<QString, QVariant> DatabaseQtSQL::GetConfiguration(user_id_t user)
{
    QSqlQuery q(this->db);
    q.prepare("SELECT value FROM settings WHERE user_id = :user_id;");
    q.bindValue(":user_id", user);

    if (!q.exec())
    {
        throw new Exception("Unable to retrieve user settings from sql: " + q.lastError().text(), BOOST_CURRENT_FUNCTION);
    }
    if (q.size() == 0)
    {
        return QHash<QString, QVariant>();
    }
    q.first();
    QHash<QString, QVariant> hash = FromArray(q.value(0).toByteArray());
    return hash;
}

void DatabaseQtSQL::SetConfiguration(user_id_t user, QHash<QString, QVariant> data)
{
    // First we need to check if there is a record for this user
    QSqlQuery exists = this->db.exec("SELECT COUNT(1) FROM settings where user_id = " + QString::number(user) + ";");
    if (!exists.isActive())
        throw new Exception("Unable to store user settings to sql: " + this->db.lastError().text(), BOOST_CURRENT_FUNCTION);

    exists.first();

    int results = exists.value(0).toInt();
    if (results > 1)
    {
        throw new Exception("User " + QString::number(user) + " has more than 1 blob hash", BOOST_CURRENT_FUNCTION);
    }

    QSqlQuery q(this->db);

    if (!results)
        q.prepare("INSERT INTO settings(user_id, value) VALUES (:1, :2);");
    else
        q.prepare("UPDATE settings SET value = :2 WHERE user_id = :1;");

    q.bindValue(":1", user);
    q.bindValue(":2", ToArray(data));

    if (!q.exec())
    {
        throw new Exception("Unable to store user data to sql: " + q.lastError().text(), BOOST_CURRENT_FUNCTION);
    }
}

QHash<QString, QByteArray> DatabaseQtSQL::GetStorage(user_id_t user)
{
    QHash<QString, QByteArray> result;
    QSqlQuery q(this->db);
    q.prepare("SELECT key, data FROM user_data WHERE user_id = :1;");
    q.bindValue(":1", user);
    if (!q.exec())
        throw new Exception("Unable to fetch user data: " + q.lastError().text(), BOOST_CURRENT_FUNCTION);

    while (q.next())
    {
        QString key = q.value(0).toString();
        QByteArray value = q.value(1).toByteArray();
        if (!result.contains(key))
            result.insert(key, value);
    }
    return result;
}

void DatabaseQtSQL::InsertStorage(user_id_t user, QString key, QByteArray data)
{
    QSqlQuery query(this->db);
    query.prepare("INSERT INTO user_data (user_id, key, data) VALUES (:1, :2, :3);");
    query.bindValue(":1", user);
    query.bindValue(":2", key);
    query.bindValue(":3", data);
    if (!query.exec())
    {
        throw new Exception("Unable to store user data: " + query.lastError().text(), BOOST_CURRENT_FUNCTION);
    }
}

void DatabaseQtSQL::UpdateStorage(user_id_t user, QString key, QByteArray data)
{
    QSqlQuery query(this->db);
    query.prepare("UPDATE user_data SET data = :3 WHERE user_id = :1 AND key = :2;");
    query.bindValue(":1", user);
    query.bindValue(":2", key);
    query.bindValue(":3", data);
    if (!query.exec())
    {
        throw new Exception("Unable to update user data: " + query.lastError().text(), BOOST_CURRENT_FUNCTION);
    }
}

void DatabaseQtSQL::RemoveStorage(user_id_t user, QString key)
{
    QSqlQuery query(this->db);
    query.prepare("DELETE FROM user_data WHERE user_id = :1 AND key = :2;");
    query.bindValue(":1", user);
    query.bindValue(":2", key);
    if (!query.exec())
    {
        throw new Exception("Unable to remove user data: " + query.lastError().text(), BOOST_CURRENT_FUNCTION);
    }
}

void DatabaseQtSQL::OffloadSQL(QString sql, bool crash_on_fail)
{
    bool result = this->ExecuteNonQuery(sql);
    if (crash_on_fail && !result)
        throw new Exception("Offloaded SQL failed: " + this->db.lastError().text(), BOOST_CURRENT_FUNCTION);
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
    GRUMPY_ERROR("SQL failed: " + reason);
    this->isFailed = true;
    this->failureReason = reason;
}

