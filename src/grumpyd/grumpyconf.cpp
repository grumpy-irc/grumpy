//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "grumpyconf.h"
#include "../libcore/configuration.h"
#include "corewrapper.h"
#include "../libcore/core.h"

using namespace GrumpyIRC;

GrumpyConf *GrumpyConf::Conf = NULL;

GrumpyConf::GrumpyConf()
{
    this->StorageDummy = false;
    this->Daemon = false;
}

Configuration *GrumpyConf::GetConfiguration()
{
    return GCFG;
}

QString GrumpyConf::GetQuitMessage()
{
    QString qm = GCFG->GetValueAsString("quit_message", "Grumpy IRC v. $version");
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

QString GrumpyConf::GetStorage()
{
    if (this->StorageDummy)
        return "DatabaseDummy";
    return GCFG->GetValueAsString("storage", "DatabaseBin");
}

void GrumpyConf::SetStorage(QString name)
{
    GCFG->SetValue("storage", QVariant(name));
}





