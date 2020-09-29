//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "databaseqtsqlite.h"
#include "grumpyd.h"
#include "grumpyconf.h"
#include "../libcore/core.h"
#include "../libcore/eventhandler.h"
#include <QtSql>
#include <QFile>

using namespace GrumpyIRC;

DatabaseQtSqlite::DatabaseQtSqlite()
{
    this->datafile = Grumpyd::GetDFPath() + "sqlite.dat";
    bool install = !QFile::exists(this->datafile);
    this->db = QSqlDatabase::addDatabase("SQLITE");
    this->db.setDatabaseName(this->datafile);
    if (!this->db.open())
        throw new Exception("Failed to open SQL storage: " + this->db.lastError().text(), BOOST_CURRENT_FUNCTION);

    if (install)
    {
        this->install();
    } else
    {
        this->init();
    }
}

void DatabaseQtSqlite::init()
{
    // Check if datafile is OK
    QSqlQuery q;
    if (!q.exec("SELECT value FROM meta WHERE key = 'version';"))
    {
        throw new Exception("The database file is corrupted: " + q.lastError().text(), BOOST_CURRENT_FUNCTION);
    }
    if (!q.first())
    {
        throw new Exception("The database file is corrupted, no version info in meta table", BOOST_CURRENT_FUNCTION);
    }

    unsigned int version = q.value(0).toUInt();
    if (version < GRUMPYD_SCHEMA_VERSION)
    {
        GRUMPY_LOG("Database is outdated (version " + QString::number(version) + ") updating to " + QString::number(GRUMPYD_SCHEMA_VERSION));
        while (version++ < GRUMPYD_SCHEMA_VERSION)
            this->UpdateDB(version);

    } else if (version > GRUMPYD_SCHEMA_VERSION)
    {
        GRUMPY_LOG("Database seems to belong to grumpyd that is newer than this version, it may not work");
    }
}

bool DatabaseQtSqlite::install()
{
    QString error;
    if (!this->ExecuteFile(GetSource("install.sql"), &error))
        throw new Exception("Failed to initialize SQL storage: " + error, BOOST_CURRENT_FUNCTION);

    // Let's set grumpy to init mode
    CONF->Init = true;
    return true;
}

void DatabaseQtSqlite::UpdateDB(unsigned int patch)
{
    GRUMPY_LOG("Updating schema to version " + QString::number(patch));
    QString error;
    if (!this->ExecuteFile(GetSource("patch_sql_" + QString::number(patch) + ".sql"), &error))
    {
        throw new Exception("Failed to upgrade DB to version " + QString::number(patch) + ", error: " + error, BOOST_CURRENT_FUNCTION);
    }
}
