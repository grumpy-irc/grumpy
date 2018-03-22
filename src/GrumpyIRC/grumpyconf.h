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

#include <QVariant>
#include <QNetworkProxy>
#include "../libirc/libircclient/network.h"

namespace libircclient
{
    class User;
}

namespace GrumpyIRC
{
    class Configuration;
    class GrumpyConf
    {
        public:
            static GrumpyConf *Conf;
            GrumpyConf();
            Configuration *GetConfiguration();
            QString GetQuitMessage();
            QString GetRawQuitMessage();
            void SetQuitMessage(QString text);
            void SetNick(QString nick);
            QString GetIdent();
            void SetIdent(QString ident);
            QString GetAlterNick();
            void SetAlterNick(QString text);
            bool WriteNoticesToSystem();
            void SetName(QString text);
            QList<char> GetSeparators();
            bool IsDisabledMessage(QString id);
            void SetDisabledMessage(bool enabled, QString id);
            QString GetName();
            QString GetNick();
            QString GetLineFormat();
            void SetLineFormat(QString format);
            void SetNoticeFormat(QString format);
            void SetMessageFormat(QString format);
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
            void SetAutorun(QString data);
            void SetLabeledH(QString text);
            void SetStandardH(QString text);
            void SetChannelH(QString text);
            QString GetLabeledHeader() const;
            QString GetChannelHeader() const;
            QString GetStandardHeader() const;
            void SetActionFormat(QString format);
            float GetTransparency();
            void SetIgnoreSSLProblems(bool set);
            bool GetIgnoreSSLProblems();
            void SetDefaultAwayReason(QString reason);
            bool Batches();
            void SetDefaultKickReason(QString text);
            void SetSplitMaxSize(int size);
            int GetSplitMaxSize();
            void SetSplit(bool split);
            unsigned int GetBatchMaxSize();
            bool GetSplit();
            bool FirstRun();
            QList<int> IgnoredNums();
            void SetIRCIgnoredNumerics(QList<int> list);
            QString GetDefaultBanMask();
            void SetDefaultBanMask(QString ban);
            QString GetMaskForUser(libircclient::User user);
            QString GetMaskForUser(libircclient::User *user);

            /////////////////////////////////////////////////////////////////////////
            // Proxy
            void SetProxy(int proxy);
            void SetProxy(QNetworkProxy proxy);
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
