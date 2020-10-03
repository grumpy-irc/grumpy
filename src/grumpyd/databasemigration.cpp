//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2020

#include "grumpyd.h"
#include "databasebackend.h"
#include "databasemigration.h"
#include "databaseqtsqlite.h"
#include "databaseqtpsql.h"
#include "grumpyconf.h"
#include "syncableircsession.h"
#include "virtualscrollback.h"
#include "user.h"
#include "userconfiguration.h"
#include "session.h"
#include "../libcore/eventhandler.h"
#include "../libcore/exception.h"
#include "../libcore/core.h"
#include "../libcore/generic.h"

using namespace GrumpyIRC;

void DatabaseMigration::SQLite2PSQL()
{
    GRUMPY_LOG("Migrating database from SQLite backend to PostgreSQL...");
    DatabaseBackend *target = Grumpyd::GetBackend();
    DatabaseBackend *source = new DatabaseQtSqlite();
    DatabaseMigration::migrate(source, target);
}

void DatabaseMigration::PSQL2SQLite()
{
    GRUMPY_LOG("Migrating database from PostgreSQL backend to SQLite...");
    DatabaseBackend *target = Grumpyd::GetBackend();
    DatabaseBackend *source = new DatabaseQtPsql();
    DatabaseMigration::migrate(source, target);
}

void DatabaseMigration::migrate(DatabaseBackend *source, DatabaseBackend *target)
{
    // Check if target is not same as source
    if (target->GetType() == source->GetType())
    {
        GRUMPY_ERROR("Target database is same type as source database, refer to https://github.com/grumpy-irc/grumpy/wiki/Grumpyd for more help");
        goto exit;
    }

    if (target->GetUserCount() > 0)
    {
        GRUMPY_ERROR("Target database already contains some user accounts, make sure target database is empty");
        goto exit;
    }

    Grumpyd::grumpyd->OverrideBackend(source);

    GRUMPY_LOG("Loading data from source database");
    source->LoadRoles();
    source->LoadUsers();
    source->LoadWindows();
    source->LoadSessions();
    source->LoadText();

    foreach (User *user, User::UserInfo)
    {
        GRUMPY_LOG("Migrating user: " + user->GetName())
        target->StoreUser(user);
        target->SetConfiguration(user->GetID(), user->GetConfiguration()->ToHash());
        QList<QString> storage_items = user->StorageList();
        foreach (QString item, storage_items)
        {
            GRUMPY_LOG("Migrating user: " + user->GetName() + " / storage: " + item);
            target->InsertStorage(user->GetID(), item, user->StorageGet(item));
        }

        foreach (IRCSession *session, user->GetSIRCSessions())
        {
            GRUMPY_LOG("Migrating user: " + user->GetName() + " / session: " + session->GetName())
            target->StoreNetwork(session);
        }

        foreach (Scrollback *scrollback, user->GetScrollbacks())
        {
            GRUMPY_LOG("Migrating user: " + user->GetName() + " / scrollback: " + scrollback->GetTarget());
            target->StoreScrollback(user, scrollback);

            QList<ScrollbackItem> items = scrollback->GetItems();

            foreach (ScrollbackItem item, items)
                target->StoreItem(user, scrollback, &item);
        }
    }

    GRUMPY_LOG("DB migration finished successfuly");

    exit:
        delete source;
}
