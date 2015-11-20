//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QFile>
#include <QSslSocket>
#include <QDir>
#include <QCoreApplication>
#include "grumpyd.h"
#include "corewrapper.h"
#include "grumpyconf.h"
#include "databasedummy.h"
#include "databaselite.h"
#include "sleeper.h"
#include "listener.h"
#include "../libcore/core.h"
#include "../libcore/configuration.h"
#include "../libcore/eventhandler.h"
#include "../libgp/gp.h"

using namespace GrumpyIRC;

Grumpyd *Grumpyd::grumpyd = NULL;

QString Grumpyd::GetCFPath()
{
    QString cf = QCoreApplication::applicationDirPath() + "/etc/";
    if (!QDir().exists(cf))
        QDir().mkpath(cf);
    return cf;
}

QString Grumpyd::GetDFPath()
{
    QString cf = QCoreApplication::applicationDirPath() + "/var/";
    if (!QDir().exists(cf))
        QDir().mkpath(cf);
    return cf;
}

QString Grumpyd::GetPathSSLCert()
{
    //! TODO load this from config
    return GetCFPath() + "cert.crt";
}

QString Grumpyd::GetPathSSLKey()
{
    return GetCFPath() + "privkey.pem";
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
}

Grumpyd::~Grumpyd()
{
    delete this->listenerSSL;
    delete this->listener;
}

DatabaseBackend *Grumpyd::GetBackend()
{
    return grumpyd->databaseBackend;
}

static DatabaseBackend *InstantiateStorage(QString type)
{
    if (type == "DatabaseDummy")
        return new DatabaseDummy();
#ifdef GRUMPYD_SQLITE
    else if (type == "DatabaseLite")
        return new DatabaseLite();
#endif
    else
        throw new Exception("Unknown database: " + type, BOOST_CURRENT_FUNCTION);
}

void Grumpyd::Main()
{
    GRUMPY_LOG("Loading storage: " + CONF->GetStorage());
    this->databaseBackend = InstantiateStorage(CONF->GetStorage());
    this->databaseBackend->LoadRoles();
    this->databaseBackend->LoadUsers();
    this->databaseBackend->LoadWindows();
    this->databaseBackend->LoadSessions();
    this->databaseBackend->LoadText();
    GRUMPY_LOG("Starting listeners");
    if (!this->listener->listen(QHostAddress::Any, GP_DEFAULT_PORT))
    {
        GRUMPY_ERROR("Unable to open listener on port " + QString::number(GP_DEFAULT_PORT));
    }
    else
    {
        GRUMPY_LOG("Listener open on port " + QString::number(GP_DEFAULT_PORT));
    }
    if (SSLIsAvailable())
    {
        if (!this->listenerSSL->listen(QHostAddress::Any, GP_DEFAULT_SSL_PORT))
        {
            GRUMPY_ERROR("Unable to open listener (SSL) on port " + QString::number(GP_DEFAULT_SSL_PORT));
        }
        else
        {
            GRUMPY_LOG("Listener (SSL) open on port " + QString::number(GP_DEFAULT_SSL_PORT));
        }
    }
}

