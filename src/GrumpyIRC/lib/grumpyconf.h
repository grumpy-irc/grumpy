//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef GRUMPYCONF_H
#define GRUMPYCONF_H

#ifdef GCFG
    #undef GCFG
#endif

#define GCFG CoreWrapper::GrumpyCore->GetConfiguration()

#ifdef CONF
#error "CONF is already defined, redefining is not supported, grumpy can't be compiled with libraries that define their own CONF option"
#endif
#define CONF GrumpyIRC::GrumpyConf::Conf

#include "grumpy_global.h"
#include <QVariant>
#include <QNetworkProxy>
#include <libirc/libircclient/network.h>

namespace libircclient
{
    class User;
}

namespace GrumpyIRC
{
    class Configuration;
    class LIBGRUMPYSHARED_EXPORT GrumpyConf
    {
        public:
            static GrumpyConf *Conf;
            GrumpyConf();
            Configuration *GetConfiguration();
            QString GetQuitMessage();
            QString GetRawQuitMessage();
            void SetQuitMessage(const QString &text);
            void SetNick(const QString &nick);
            QString GetIdent();
            void SetIdent(const QString &ident);
            QString GetAlterNick();
            void SetAlterNick(const QString &text);
            bool WriteNoticesToSystem();
            void SetName(const QString &text);
            QList<char> GetSeparators();
            bool IsDisabledMessage(const QString &id);
            void SetDisabledMessage(bool enabled, const QString &id);
            QString GetName();
            QString GetNick();
            QString GetLineFormat();
            void SetLineFormat(const QString &format);
            void SetNoticeFormat(const QString &format);
            void SetMessageFormat(const QString &format);
            void SetEncoding(libircclient::Encoding encoding);
            libircclient::Encoding GetEncoding();
            QString GetNoticeFormat();
            QString GetMessageFormat();
            QString GetActionFormat();
            QString GetDefaultKickReason();
            QString GetDefaultAwayReason();
            QString GetAutorun();
            void SetColorBoxShow(bool yes);
            bool GetColorBoxShow();
            void SetAutorun(const QString &data);
            void SetLabeledH(const QString &text);
            void SetStandardH(const QString &text);
            void SetChannelH(const QString &text);
            QString GetLabeledHeader() const;
            QString GetChannelHeader() const;
            QString GetStandardHeader() const;
            void SetActionFormat(const QString &format);
            float GetTransparency();
            void SetIgnoreSSLProblems(bool set);
            bool GetIgnoreSSLProblems();
            void SetDefaultAwayReason(const QString &reason);
            bool Batches();
            void SetDefaultKickReason(const QString &text);
            void SetSplitMaxSize(int size);
            int GetSplitMaxSize();
            void SetSplit(bool split);
            unsigned int GetBatchMaxSize();
            bool GetSplit();
            bool FirstRun();
            QList<int> IgnoredNums();
            void SetIRCIgnoredNumerics(const QList<int> &list);
            QString GetDefaultBanMask();
            void SetDefaultBanMask(const QString &ban);
            QString GetMaskForUser(libircclient::User user);
            QString GetMaskForUser(libircclient::User *user);
            bool GetAutoReduceMaxSendSize();
            void SetAutoReduceMaxSendSize(bool yes);
            void SetAutoAwayMsg(const QString &message);
            QString GetAutoAwayMsg();
            void SetAutoAway(bool yes);
            bool GetAutoAway();
            int GetAutoAwayTime();
            void SetAutoAwayTime(int time);
            void SetLastSavePath(const QString &path);
            QString GetLastSavePath();

            /////////////////////////////////////////////////////////////////////////
            // Proxy
            void SetProxy(int proxy);
            void SetProxy(const QNetworkProxy &proxy);
            bool UsingProxy();
            int ProxyType();
            QNetworkProxy GetProxy();

            /////////////////////////////////////////////////////////////////////////

            void Load();
            void Save();
            bool SafeMode = false;
            unsigned int Verbosity = 0;
    };
}

#endif // GRUMPYCONF_H
