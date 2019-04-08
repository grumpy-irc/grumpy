//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef HOOKS_H
#define HOOKS_H

#include "libcore_global.h"
#include "definitions.h"

namespace libircclient
{
    class Parser;
}

namespace GrumpyIRC
{
    class Scrollback;
    class ScrollbackItem;
    class IRCSession;

    class LIBCORESHARED_EXPORT Hooks
    {
        public:
            static void OnScrollback_InsertText(Scrollback *scrollback, ScrollbackItem *item);
            static void OnScrollback_Destroyed(Scrollback *scrollback);
            static void OnNetwork_Disconnect(IRCSession *session);
            static void OnNetwork_Generic(IRCSession *session, libircclient::Parser *px);
            static void OnNetwork_UnknownMessage(IRCSession *session, libircclient::Parser *px);
            static void OnNetwork_ChannelJoined(IRCSession *session, const QString& channel_name);
            static void OnNetwork_ChannelParted(IRCSession *session, libircclient::Parser *px, const QString& channel_name, const QString &reason);
            static void OnNetwork_ChannelLeft(IRCSession *session, libircclient::Parser *px, const QString& channel_name, const QString &reason);
            static void OnNetwork_ChannelKicked(IRCSession *session, libircclient::Parser *px, const QString& channel_name, const QString &reason);
    };
}

#endif // HOOKS_H
