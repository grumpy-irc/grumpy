//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "corewrapper.h"
#include "grumpyeventhandler.h"
#include "skin.h"
#include "grumpyconf.h"
#include <libcore/generic.h>
#include <libcore/highlighter.h>
#include <libcore/configuration.h>
#include <libcore/core.h>
#include <libirc/libircclient/user.h>
#include <QFile>

using namespace GrumpyIRC;

GrumpyConf *GrumpyConf::Conf = nullptr;

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
    return GCFG->GetValueAsString("quit_message", "GrumpyChat v. $version. Such client. WOW. Much quit. https://github.com/grumpy-irc/grumpy");
}

void GrumpyConf::SetQuitMessage(const QString &text)
{
    GCFG->SetValue("quit_message", QVariant(text));
}

void GrumpyConf::SetNick(const QString &nick)
{
    GCFG->SetValue("nick", QVariant(nick));
}

QString GrumpyConf::GetIdent()
{
    return GCFG->GetValueAsString("ident", DEFAULT_IDENT);
}

void GrumpyConf::SetIdent(const QString &ident)
{
    GCFG->SetValue("ident", QVariant(ident));
}

QString GrumpyConf::GetAlterNick()
{
    return GCFG->GetValueAsString("alternative_nick");
}

void GrumpyConf::SetAlterNick(const QString &text)
{
    GCFG->SetValue("alternative_nick", QVariant(text));
}

bool GrumpyConf::WriteNoticesToSystem()
{
    return GCFG->GetValueAsBool("write_notices_to_system", true);
}

void GrumpyConf::SetName(const QString &text)
{
    GCFG->SetValue("name", text);
}

QList<char> GrumpyConf::GetSeparators()
{
    QList<char> results;
    results << '(' << ')' << ' ' << '.' << ',' << ';' << '!';
    return results;
}

bool GrumpyConf::IsDisabledMessage(const QString &id)
{
    if (!GCFG->Contains("messages.disabled." + id))
        return false;
    return GCFG->GetValueAsBool("messages.disabled." + id);
}

void GrumpyConf::SetDisabledMessage(bool enabled, const QString &id)
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

void GrumpyConf::SetLineFormat(const QString &format)
{
    GCFG->SetValue("line_format", format);
}

void GrumpyConf::SetNoticeFormat(const QString &format)
{
    GCFG->SetValue("notice_format", format);
}

void GrumpyConf::SetMessageFormat(const QString &format)
{
    GCFG->SetValue("message_format", format);
}

void GrumpyConf::SetEncoding(libircclient::Encoding encoding)
{
    GCFG->SetValue("encoding", static_cast<int> (encoding));
}

libircclient::Encoding GrumpyConf::GetEncoding()
{
    return static_cast<libircclient::Encoding> (GCFG->GetValueAsInt("encoding"));
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
    QFile f(GCFG->GetHomePath() + "autoexec.cfg");
    if (!f.open(QIODevice::ReadOnly))
    {
        f.close();
        return default_autorun;
    }
    QString content = f.readAll();
    f.close();
    return content;
}

void GrumpyConf::SetColorBoxShow(bool yes)
{
    GCFG->SetValue("color_box_show", yes);
}

bool GrumpyConf::GetColorBoxShow()
{
    return GCFG->GetValueAsBool("color_box_show", true);
}

void GrumpyConf::SetAutorun(const QString &data)
{
    QFile f(GCFG->GetHomePath() + "autoexec.cfg");

    if (!f.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        f.close();
        GRUMPY_ERROR("Unable to write autoexec file: " + GCFG->GetHomePath() + "autoexec.cfg");
        return;
    }

    f.write(data.toUtf8());
    f.close();
}

void GrumpyConf::SetLabeledH(const QString &text)
{
    GCFG->SetValue("labeled_header", text);
}

void GrumpyConf::SetStandardH(const QString &text)
{
    GCFG->SetValue("standard_header", text);
}

void GrumpyConf::SetChannelH(const QString &text)
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

void GrumpyConf::SetActionFormat(const QString &format)
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

void GrumpyConf::SetDefaultAwayReason(const QString &reason)
{
    GCFG->SetValue("reason_away", reason);
}

bool GrumpyConf::Batches()
{
    return GCFG->GetValueAsBool("mode_batching", true);
}

void GrumpyConf::SetDefaultKickReason(const QString &text)
{
    GCFG->SetValue("kick", text);
}

void GrumpyConf::SetSplitMaxSize(int size)
{
    GCFG->SetValue("split_max_word_size", size);
}

int GrumpyConf::GetSplitMaxSize()
{
    return GCFG->GetValueAsInt("split_max_word_size", 512);
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

void GrumpyConf::SetIRCIgnoredNumerics(const QList<int> &list)
{
    GCFG->SetValue("ignored_nmrs", Generic::QIntListToVariantList(list));
}

QString GrumpyConf::GetDefaultBanMask()
{
    return GCFG->GetValueAsString("ban_mask", "*!*@$host");
}

void GrumpyConf::SetDefaultBanMask(const QString &ban)
{
    GCFG->SetValue("ban_mask", ban);
}

QString GrumpyConf::GetMaskForUser(libircclient::User user)
{
    return this->GetMaskForUser(&user);
}

QString GrumpyConf::GetMaskForUser(libircclient::User *user)
{
    return this->GetDefaultBanMask().replace("$nick", user->GetNick())
                                    .replace("$ident", user->GetIdent())
            .replace("$host", user->GetHost());
}

bool GrumpyConf::GetAutoReduceMaxSendSize()
{
    return GCFG->GetValueAsBool("auto_reduce_max_send_size", true);
}

void GrumpyConf::SetAutoReduceMaxSendSize(bool yes)
{
    GCFG->SetValue("auto_reduce_max_send_size", yes);
}

void GrumpyConf::SetAutoAwayMsg(const QString &message)
{
    GCFG->SetValue("auto_away_text", message);
}

QString GrumpyConf::GetAutoAwayMsg()
{
    return GCFG->GetValueAsString("auto_away_text", "I am currently AFK");
}

void GrumpyConf::SetAutoAway(bool yes)
{
    GCFG->SetValue("auto_away", yes);
}

bool GrumpyConf::GetAutoAway()
{
    return GCFG->GetValueAsBool("auto_away", true);
}

int GrumpyConf::GetAutoAwayTime()
{
    return GCFG->GetValueAsInt("auto_away_time", 60 * 10);
}

void GrumpyConf::SetAutoAwayTime(int time)
{
    if (time < 10)
        time = 10;
    GCFG->SetValue("auto_away_time", time);
}

void GrumpyConf::SetLastSavePath(const QString &path)
{
    GCFG->SetValue("save_path", path);
}

QString GrumpyConf::GetLastSavePath()
{
    return GCFG->GetValueAsString("save_path");
}

bool GrumpyConf::GetContinuousLoggingEnabled()
{
    return GCFG->GetValueAsBool("continuous_logging");
}

void GrumpyConf::SetContinuousLoggingEnabled(bool enabled)
{
    GCFG->SetValue("continuous_logging", enabled);
}

QString GrumpyConf::GetContinuousLoggingPath()
{
    return GCFG->GetValueAsString("continuous_logging_path", GCFG->GetHomePath() + "logs/");
}

void GrumpyConf::SetContinuousLoggingPath(QString path)
{
    GCFG->SetValue("continuous_logging_path", path);
}

QHash<QString, QVariant> GrumpyConf::GetIdentities()
{
    return GCFG->GetValueAsHash("identities");
}

QHash<QString, QVariant> GrumpyConf::GetNetworks()
{
    return GCFG->GetValueAsHash("networks");
}

void GrumpyConf::SetIdentities(const QHash<QString, QVariant> &list)
{
    GCFG->SetValue("identities", list);
}

void GrumpyConf::SetNetworks(const QHash<QString, QVariant> &hash)
{
    GCFG->SetValue("networks", hash);
}

void GrumpyConf::SetProxy(int proxy)
{
    GCFG->SetValue("proxy", proxy);
}

void GrumpyConf::SetProxy(const QNetworkProxy &proxy)
{
    GCFG->SetValue("proxy_hostname", proxy.hostName());
    GCFG->SetValue("proxy_user", proxy.user());
    GCFG->SetValue("proxy_password", proxy.password());
    GCFG->SetValue("proxy_port", proxy.port());
}

bool GrumpyConf::UsingProxy()
{
    return GCFG->GetValueAsInt("proxy") != 0;
}

int GrumpyConf::ProxyType()
{
    return GCFG->GetValueAsInt("proxy");
}

QNetworkProxy GrumpyConf::GetProxy()
{
    QNetworkProxy p;
    p.setHostName(GCFG->GetValueAsString("proxy_hostname", "127.0.0.1"));
    p.setPassword(GCFG->GetValueAsString("proxy_password"));
    p.setUser(GCFG->GetValueAsString("proxy_user"));
    p.setPort(GCFG->GetValueAsInt("proxy_port", 0));
    return p;
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
    GCFG->SetValue("skin", static_cast<int>(Skin::SkinList.indexOf(Skin::GetCurrent())));
    GCFG->Save();
}

