//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "corewrapper.h"
#include "highlighter.h"
#include "grumpyconf.h"
#include "../libcore/configuration.h"
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
    QString qm = this->GetRawQuitMessage();
    qm.replace("$version", GRUMPY_VERSION_STRING);
    return qm;
}

QString GrumpyConf::GetRawQuitMessage()
{
    return GCFG->GetValueAsString("quit_message", "GrumpyIRC v. $version. Such client. WOW. Much quit.");
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

void GrumpyConf::SetName(QString text)
{
    GCFG->SetValue("name", text);
}

QString GrumpyConf::GetName()
{
    return GCFG->GetValueAsString("name", "Grumpy IRC");
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

QString GrumpyConf::GetDefaultKickReason()
{
    return GCFG->GetValueAsString("kick", "Such user. Much kicked.");
}

void GrumpyConf::SetDefaultKickReason(QString text)
{

}

bool GrumpyConf::FirstRun()
{
    return GCFG->GetValueAsBool("first_run", true);
}

void GrumpyConf::Load()
{
    GCFG->Load();
    if (GCFG->Contains("highlights"))
    {
        qDeleteAll(Highlighter::Highlighter_Data);
        QList<QVariant> list = GCFG->GetValue("highlights").toList();
        foreach (QVariant item, list)
        {
            new Highlighter(item.toHash());
        }
    }
}

void GrumpyConf::Save()
{
    GCFG->SetValue("first_run", false);
    QList<QVariant> ql;
    foreach (Highlighter *item, Highlighter::Highlighter_Data)
    {
        ql.append(item->ToHash());
    }
    GCFG->SetValue("highlights", QVariant(ql));
    GCFG->Save();
}

