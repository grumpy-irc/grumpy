//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "../libcore/definitions.h"
#include "../libcore/exception.h"
#include "../libcore/generic.h"
#include "userconfiguration.h"
#include "grumpyd.h"
#include "databasebackend.h"

using namespace GrumpyIRC;

UserConf::UserConf(user_id_t user)
{
    this->User = user;
}

void UserConf::Save()
{
    Grumpyd::grumpyd->GetBackend()->SetConfiguration(this->User, this->Options);
}

void UserConf::Load()
{
    this->Options = Grumpyd::grumpyd->GetBackend()->GetConfiguration(this->User);
}

void UserConf::SetHash(QHash<QString, QVariant> hash)
{
    this->Options = hash;
}

QList<int> UserConf::IgnoredNums()
{
    if (!this->Contains("ignored_nmrs"))
    {
        QList<int> list;
        // These are really annoying and not very useful
        list << 305 << 306;
        return list;
    }
    return Generic::QVariantListToIntList(this->GetValue("ignored_nmrs").toList());
}

void UserConf::SetIRCIgnoredNumerics(QList<int> list)
{
    this->SetValue("ignored_nmrs", Generic::QIntListToVariantList(list));
}

QHash<QString, QVariant> UserConf::ToHash()
{
    return this->Options;
}

