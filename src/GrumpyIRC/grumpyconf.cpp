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
#include "skin.h"
#include "../libcore/generic.h"
#include "../libcore/highlighter.h"
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
    return GCFG->GetValueAsString("quit_message", "GrumpyChat v. $version. Such client. WOW. Much quit.");
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

QList<char> GrumpyConf::GetSeparators()
{
    QList<char> results;
    results << '(' << ')' << ' ' << '.' << ',' << ';' << '!';
    return results;
}

bool GrumpyConf::IsDisabledMessage(QString id)
{
    if (!GCFG->Contains("messages.disabled." + id))
        return false;
    return GCFG->GetValueAsBool("messages.disabled." + id);
}

void GrumpyConf::SetDisabledMessage(bool enabled, QString id)
{
    GCFG->SetValue("messages.disabled." + id, enabled);
}

QString GrumpyConf::GetName()
{
    return GCFG->GetValueAsString("name", "Grumpy Chat");
}

QString GrumpyConf::GetNick()
{
    return GCFG->GetValueAsString("nick", "GrumpyUser");
}

QString GrumpyConf::GetLineFormat()
{
    return GCFG->GetValueAsString("line_format", "($time) $string");
}

void GrumpyConf::SetLineFormat(QString format)
{
    GCFG->SetValue("line_format", format);
}

void GrumpyConf::SetNoticeFormat(QString format)
{
    GCFG->SetValue("notice_format", format);
}

void GrumpyConf::SetMessageFormat(QString format)
{
    GCFG->SetValue("message_format", format);
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

QString GrumpyConf::GetDefaultAwayReason()
{
    return GCFG->GetValueAsString("reason_away", "I am currently not here");
}

QString GrumpyConf::GetAutorun()
{
    QString default_autorun = Generic::GetResource(":/text/scripts/_autoexec");
    return GCFG->GetValueAsString("execute_autorun", default_autorun);
}

void GrumpyConf::SetAutorun(QString data)
{
    GCFG->SetValue("execute_autorun", data);
}

void GrumpyConf::SetLabeledH(QString text)
{
    GCFG->SetValue("labeled_header", text);
}

void GrumpyConf::SetStandardH(QString text)
{
    GCFG->SetValue("standard_header", text);
}

void GrumpyConf::SetChannelH(QString text)
{
    GCFG->SetValue("channel_header", text);
}

QString GrumpyConf::GetLabeledHeader() const
{
    return GCFG->GetValueAsString("labeled_header", "GrumpyChat: $title");
}

QString GrumpyConf::GetChannelHeader() const
{
    return GCFG->GetValueAsString("channel_header", "GrumpyChat - $title: $topic");
}

QString GrumpyConf::GetStandardHeader() const
{
    return GCFG->GetValueAsString("standard_header", "GrumpyChat");
}

void GrumpyConf::SetActionFormat(QString format)
{
    GCFG->SetValue("action_format", format);
}

float GrumpyConf::GetTransparency()
{
    return GCFG->GetValueAsFloat("transparency_main", 0);
}

void GrumpyConf::SetIgnoreSSLProblems(bool set)
{
    GCFG->SetValue("ignore_ssl", set);
}

bool GrumpyConf::GetIgnoreSSLProblems()
{
    return GCFG->GetValueAsBool("ignore_ssl", true);
}

void GrumpyConf::SetDefaultAwayReason(QString reason)
{
    GCFG->SetValue("reason_away", reason);
}

bool GrumpyConf::Batches()
{
    return GCFG->GetValueAsBool("mode_batching", true);
}

void GrumpyConf::SetDefaultKickReason(QString text)
{
    GCFG->SetValue("kick", text);
}

void GrumpyConf::SetSplitMaxSize(int size)
{
    GCFG->SetValue("split_max_word_size", size);
}

int GrumpyConf::GetSplitMaxSize()
{
    return GCFG->GetValueAsInt("split_max_word_size", 200);
}

void GrumpyConf::SetSplit(bool split)
{
    GCFG->SetValue("split", split);
}

unsigned int GrumpyConf::GetBatchMaxSize()
{
    return GCFG->GetValueAsUInt("maximum_irc_batch_size", 4);
}

bool GrumpyConf::GetSplit()
{
    return GCFG->GetValueAsBool("split", true);
}

bool GrumpyConf::FirstRun()
{
    return GCFG->GetValueAsBool("first_run", true);
}

QList<int> GrumpyConf::IgnoredNums()
{
    if (!GCFG->Contains("ignored_nmrs"))
    {
        QList<int> list;
        // These are really annoying and not very useful
        list << 305 << 306;
        return list;
    }
    return Generic::QVariantListToIntList(GCFG->GetValue("ignored_nmrs").toList());
}

void GrumpyConf::SetIRCIgnoredNumerics(QList<int> list)
{
    GCFG->SetValue("ignored_nmrs", Generic::QIntListToVariantList(list));
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

    // This will actually load the default skin in case it wasn't already, we need that to ensure that it's first in list
    Skin::GetDefault();

    if (GCFG->Contains("skins"))
    {
        Skin::Clear();
        QList<QVariant> list = GCFG->GetValue("skins").toList();
        foreach (QVariant item, list)
        {
            new Skin(item.toHash());
        }
    }
    if (GCFG->Contains("skin"))
    {
        int skin = GCFG->GetValueAsInt("skin");
        if (skin < 0 || skin >= Skin::SkinList.count())
            return;

        Skin::Current = Skin::SkinList[skin];
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
    QList<QVariant> sl;
    foreach (Skin *sx, Skin::SkinList)
    {
        if (sx->IsDefault())
            continue;
        sl.append(sx->ToHash());
    }
    GCFG->SetValue("highlights", ql);
    if (!sl.isEmpty())
        GCFG->SetValue("skins", sl);
    GCFG->SetValue("skin", Skin::SkinList.indexOf(Skin::GetCurrent()));
    GCFG->Save();
}

