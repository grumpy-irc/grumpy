//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include <QFile>
#include <QSslSocket>
#include <QDir>
#include <QCoreApplication>
#include "grumpyd.h"
#include "corewrapper.h"
#include "grumpyconf.h"
#include "databasedummy.h"
#include "databaseqtsqlite.h"
#include "user.h"
#include "sleeper.h"
#include "session.h"
#include "listener.h"
#include "databaseqtsql.h"
#include "databaseqtpsql.h"
#include "databasebin.h"
#include "databasemigration.h"
#include "script_engine/grumpydscript.h"
#include "../libcore/core.h"
#include "../libcore/configuration.h"
#include "../libcore/eventhandler.h"
#include "../libgp/gp.h"

using namespace GrumpyIRC;

Grumpyd *Grumpyd::grumpyd = nullptr;

QString Grumpyd::GetCFPath()
{
    return CONF->GetCertFilePath();
}

QString Grumpyd::GetDFPath()
{
    return CONF->GetDatafilePath();
}

QString Grumpyd::GetPathSSLCert()
{
    //! TODO load this from config
    return GetCFPath() + "server.crt";
}

QString Grumpyd::GetPathSSLKey()
{
    return GetCFPath() + "server.key";
}

bool Grumpyd::SSLIsAvailable()
{
    if (!QFile().exists(GetPathSSLKey()) || !QFile().exists(GetPathSSLCert()))
    {
        GRUMPY_ERROR("SSL not available because either SSL key or certificate couldn't be found");
        GRUMPY_ERROR("Certificate path: " + GetPathSSLCert());
        GRUMPY_ERROR("Key path: " + GetPathSSLKey());
        return false;
    }

    if (!QSslSocket::supportsSsl())
    {
        GRUMPY_ERROR("This system doesn't support SSL (missing OpenSSL libs)");
        return false;
    }

    return true;
}

Grumpyd::Grumpyd()
{
    running = true;
    grumpyd = this;
    this->listener = new Listener();
    this->listenerSSL = new Listener(true);
    this->tClean = new QTimer(this);
    connect(this->tClean, SIGNAL(timeout()), this, SLOT(MemoryCleanup()));
    this->tClean->start(10000);
}

Grumpyd::~Grumpyd()
{
    delete this->listenerSSL;
    delete this->listener;
}

void Grumpyd::Kill()
{
    foreach (User *user, User::UserInfo)
        user->Shutdown();
}

DatabaseBackend *Grumpyd::GetBackend()
{
    return grumpyd->databaseBackend;
}

void Grumpyd::OverrideBackend(DatabaseBackend *backend)
{
    if (!CONF->DBMove)
        throw new Exception("Can't override DB backend, not migrating", BOOST_CURRENT_FUNCTION);

    this->databaseBackend = backend;
}

static DatabaseBackend *InstantiateStorage(QString type)
{
    if (type == "DatabaseDummy")
    {
        return new DatabaseDummy();
    }
    else if (type == "DatabaseLite")
    {
        if (!CONF->SQLite_Enabled)
            throw new Exception("Requested database type SQLite, which is not enabled - you need to install missing Qt libraries for support of SQLite", BOOST_CURRENT_FUNCTION);
        return new DatabaseQtSqlite();
    }
    else if (type == "DatabaseBin")
    {
        return new DatabaseBin();
    }
    else if (type == "DatabasePostgre")
    {
        if (!CONF->PSQL_Enabled)
            throw new Exception("Requested database type Postgre, which is not enabled - you need to install missing Qt libraries for support of Postgre", BOOST_CURRENT_FUNCTION);
        return new DatabaseQtPsql();
    }
    else
    {
        throw new Exception("Unknown database: " + type, BOOST_CURRENT_FUNCTION);
    }
}

void Grumpyd::Main()
{
    this->initScripts();
    DatabaseQtSQL::CheckDriver();
    GRUMPY_LOG("Loading storage: " + CONF->GetStorage());
    this->databaseBackend = InstantiateStorage(CONF->GetStorage());
    if (this->databaseBackend->IsFailed())
    {
        throw new Exception("Database backend failed to initialize: " + this->databaseBackend->GetLastErrorText(), BOOST_CURRENT_FUNCTION);
        return;
    }
    if (CONF->DBMaint)
    {
        GRUMPY_LOG("Performing database maintenance");
        this->databaseBackend->Maintenance();
        QCoreApplication::exit(0);
        return;
    }
    if (CONF->DBMove)
    {
        if (CONF->DBTarget == "psql")
            DatabaseMigration::SQLite2PSQL();
        else
            DatabaseMigration::PSQL2SQLite();
        QCoreApplication::exit(0);
        return;
    }
    this->databaseBackend->LoadRoles();
    this->databaseBackend->LoadUsers();
    this->databaseBackend->LoadWindows();
    this->databaseBackend->LoadSessions();
    this->databaseBackend->LoadText();
    GRUMPY_LOG("Starting listeners");
    if (!this->listener->listen(QHostAddress::Any, CONF->DefaultPort))
    {
        GRUMPY_ERROR("Unable to open listener on port " + QString::number(CONF->DefaultPort));
    }
    else
    {
        GRUMPY_LOG("Listener open on port " + QString::number(CONF->DefaultPort));
    }
    if (SSLIsAvailable())
    {
        if (!this->listenerSSL->listen(QHostAddress::Any, CONF->SecuredPort))
        {
            GRUMPY_ERROR("Unable to open listener (SSL) on port " + QString::number(CONF->SecuredPort));
        }
        else
        {
            GRUMPY_LOG("Listener (SSL) open on port " + QString::number(CONF->SecuredPort));
        }
    }
}

void Grumpyd::MemoryCleanup()
{
    Session::DeleteOffline();
}

void Grumpyd::initScripts()
{
    QString script_path = CONF->GetScriptPath();
    QDir script_dir(script_path);
    QList<QString> scripts = script_dir.entryList();
    foreach (QString name, scripts)
    {
        if (!name.toLower().endsWith(".js"))
            continue;
        QString source;
        QString file_path = script_path + QDir::separator() + name;
        QString error;
        QFile file(file_path);
        if (!file.open(QIODevice::ReadOnly))
        {
            GRUMPY_ERROR("Unable to read source code of " + file_path);
            continue;
        }
        source = QString(file.readAll());
        file.close();
        GrumpydScript *ex = new GrumpydScript();
        if (!ex->LoadSrc(name, source, &error))
        {
            GRUMPY_ERROR("Unable to load script " + file_path + ": " + error);
            delete ex;
            return;
        }
    }
}

