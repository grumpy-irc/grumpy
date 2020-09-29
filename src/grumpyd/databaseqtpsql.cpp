//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2020

#include "databaseqtpsql.h"
#include "grumpyconf.h"
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include "../libcore/eventhandler.h"
#include "../libcore/core.h"
#include "../libcore/exception.h"

#define GRUMPYD_SCHEMA_VERSION 1

using namespace GrumpyIRC;

DatabaseQtPsql::DatabaseQtPsql()
{
    this->db = QSqlDatabase::addDatabase("QPSQL");
    this->db.setHostName(CONF->GetPSQL_Host());
    this->db.setDatabaseName(CONF->GetPSQL_Name());
    this->db.setUserName(CONF->GetPSQL_User());
    this->db.setPassword(CONF->GetPSQL_Pass());
    GRUMPY_DEBUG("PSQL: opening primary connection to server", 1);
    if (!this->db.open())
        this->fail("Unable to open connection: " + this->db.lastError().text());
    else
        this->init();
}

DatabaseQtPsql::~DatabaseQtPsql()
{
    this->db.close();
}

void DatabaseQtPsql::updateDB(unsigned int patch)
{
    GRUMPY_LOG("PSQL: Updating schema to version " + QString::number(patch));
    QString error_text;
    if (!this->ExecuteFile(GetSource("psql/patch_sql_" + QString::number(patch) + ".sql"), &error_text))
        throw new Exception("Failed to upgrade DB to version " + QString::number(patch) + ", error: " + error_text, BOOST_CURRENT_FUNCTION);
}

void DatabaseQtPsql::init()
{
    GRUMPY_DEBUG("PSQL: checking if meta table exists", 1);
    QSqlQuery result = this->db.exec("SELECT value from META where key = 'version';");
    if (!result.isActive())
    {
        GRUMPY_LOG("PSQL database contains no meta table, probably not installed? Creating database schemas now...");
        if (!this->install())
            this->fail("Unable to install database schema");
    } else if (result.size() != 1)
    {
        this->fail("DB corrupted: meta table returned unexpected amount of results for key <version> (expected 1, got " + QString::number(result.size()) + ")");
    } else
    {
        // Check database version
        QString version_info = result.value(0).toString();

        unsigned int version = version_info.toUInt();
        if (version < GRUMPYD_SCHEMA_VERSION)
        {
            GRUMPY_LOG("PSQL: Database schema is outdated (version " + QString::number(version) + ") updating to " + QString::number(GRUMPYD_SCHEMA_VERSION));
            while (version++ < GRUMPYD_SCHEMA_VERSION)
                this->updateDB(version);

        } else if (version > GRUMPYD_SCHEMA_VERSION)
        {
            GRUMPY_LOG("Database seems to belong to grumpyd that is newer than this version, it may not work");
        }
    }

    result = this->db.exec("SELECT COUNT(1) FROM users;");
    if (!result.isActive())
        this->fail("DB corrupted: Unable to select count of users");
    if (result.value(0).toInt() == 0)
    {
        GRUMPY_LOG("PSQL: table users contains no user accounts, setting grumpyd to init mode");
        CONF->Init = true;
    }
}

bool DatabaseQtPsql::install()
{
    QString error;
    if (!this->ExecuteFile(GetSource("psql/install.sql"), &error))
        throw new Exception("Failed to initialize SQL storage: " + error, BOOST_CURRENT_FUNCTION);
    return true;
}

