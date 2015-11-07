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

void GrumpyConf::SetQuitMessage(QString text)
{
    GCFG->SetValue("quit_message", QVariant(text));
}

void GrumpyConf::SetNick(QString nick)
{
    GCFG->SetValue("nick", QVariant(nick));
}

QString GrumpyConf::GetIdent()
{
    return GCFG->GetValueAsString("ident", "grumpy");
}

void GrumpyConf::SetIdent(QString ident)
{
    GCFG->SetValue("ident", QVariant(ident));
}

QString GrumpyConf::GetAlterNick()
{
    return GCFG->GetValueAsString("alternative_nick");
}

void GrumpyConf::SetAlterNick(QString text)
{
    GCFG->SetValue("alternative_nick", QVariant(text));
}

bool GrumpyConf::WriteNoticesToSystem()
{
    return GCFG->GetValueAsBool("write_notices_to_system", true);
}

QString GrumpyConf::GetNick()
{
    return GCFG->GetValueAsString("nick", "GrumpyUser");
}

QString GrumpyConf::GetLineFormat()
{
    return GCFG->GetValueAsString("line_format", "($time) $string");
}

QString GrumpyConf::GetNoticeFormat()
{
    return GCFG->GetValueAsString("notice_format", "[$nick] $string");
}

QString GrumpyConf::GetMessageFormat()
{
    return GCFG->GetValueAsString("message_format", "<$nick> $string");
}

QString GrumpyConf::GetActionFormat()
{
    return GCFG->GetValueAsString("action_format", "* $nick $string");
}

