//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include <QCoreApplication>
#include "grumpyconf.h"
#include "../libcore/configuration.h"
#include "corewrapper.h"
#include "../libcore/core.h"

using namespace GrumpyIRC;

GrumpyConf *GrumpyConf::Conf = NULL;

GrumpyConf::GrumpyConf()
{
    this->StorageDummy = false;
    this->Init = false;
    this->AutoFix = false;
    this->Daemon = false;
}

Configuration *GrumpyConf::GetConfiguration()
{
    return GCFG;
}

QString GrumpyConf::GetQuitMessage()
{
    QString qm = GCFG->GetValueAsString("quit_message", "GrumpyChat v. $version. Such bouncer. WOW. Much awesome.");
    qm.replace("$version", GRUMPY_VERSION_STRING);
    return qm;
}

void GrumpyConf::SetNick(QString nick)
{
    GCFG->SetValue("nick", QVariant(nick));
}

unsigned int GrumpyConf::GetMaxLoadSize()
{
    return 800;
}

QString GrumpyConf::GetNick()
{
    return GCFG->GetValueAsString("nick", "GrumpyUser");
}

unsigned int GrumpyConf::GetMaxScrollbackSize()
{
    return GCFG->GetValueAsUInt("max_scrollback_size", 2000);
}

unsigned int GrumpyConf::GetMaxSnifferSize()
{
    return GCFG->GetValueAsUInt("max_sniffer_size", 200);
}

unsigned int GrumpyConf::GetMaxSearchSize()
{
    return GCFG->GetValueAsUInt("max_search_size", 1000);
}

void GrumpyConf::SetMaxSearchSize(unsigned int size)
{
    GCFG->SetValue("max_search_size", size);
}

int GrumpyConf::GetPSQL_Port()
{
    return GCFG->GetValueAsInt("psql_port", 5432);
}

QString GrumpyConf::GetPSQL_Host()
{
    return GCFG->GetValueAsString("psql_host", "localhost");
}

QString GrumpyConf::GetPSQL_User()
{
    return GCFG->GetValueAsString("psql_user", "");
}

QString GrumpyConf::GetPSQL_Pass()
{
    return GCFG->GetValueAsString("psql_pass", "");
}

QString GrumpyConf::GetPSQL_Name()
{
    return GCFG->GetValueAsString("psql_name", "grumpyd");
}

QString GrumpyConf::GetStorage()
{
    if (this->StorageDummy)
        return "DatabaseDummy";

    if (!this->SQLite_Enabled)
        return GCFG->GetValueAsString("storage", "DatabaseDummy");
    else
        return GCFG->GetValueAsString("storage", "DatabaseLite");
}

void GrumpyConf::SetStorage(QString name)
{
    GCFG->SetValue("storage", name);
}

QString GrumpyConf::GetDatafilePath()
{
    return GCFG->GetValueAsString("datafile", QCoreApplication::applicationDirPath() + "/var/");
}

void GrumpyConf::SetDatafilePath(QString name)
{
    GCFG->SetValue("datafile", name);
}

QString GrumpyConf::GetCertFilePath()
{
    return GCFG->GetValueAsString("certfile", QCoreApplication::applicationDirPath() + "/etc/");
}

void GrumpyConf::SetCertFilePath(QString path)
{
    GCFG->SetValue("certfile", path);
}

QString GrumpyConf::GetScriptPath()
{
    return GCFG->GetValueAsString("scriptpath", QCoreApplication::applicationDirPath() + "/scripts/");
}

void GrumpyConf::SetScriptPath(QString path)
{
    GCFG->SetValue("scriptpath", path);
}





