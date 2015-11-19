//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

/*
#include <QFile>
#include <QDir>
#include "../libcore/exception.h"
#include "grumpyd.h"
#include "databasebin.h"
#include "security.h"

using namespace GrumpyIRC;

DatabaseBin::DatabaseBin()
{

}

void DatabaseBin::LoadRoles()
{
    Role::Defaults();
}

void DatabaseBin::LoadUsers()
{

}

QHash<QString, QVariant> DatabaseBin::GetConfiguration(user_id_t user)
{
    QHash<QString, QVariant> hash;

    return hash;
}

void DatabaseBin::SetConfiguration(user_id_t user, QHash<QString, QVariant> data)
{

}

QString DatabaseBin::getDFPath()
{
    QString path = Grumpyd::GetDFPath() + "db_bin/";
    if (!QDir().exists(path))
        QDir().mkpath(path);
    return path;
}

*/
