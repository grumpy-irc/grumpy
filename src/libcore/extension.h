//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef EXTENSION_H
#define EXTENSION_H

#include "definitions.h"
#include "libcore_global.h"
#include <QString>

namespace libircclient
{
    class Parser;
}

namespace GrumpyIRC
{
    class Scrollback;
    class IRCSession;
    class LIBCORESHARED_EXPORT Extension : public QObject
    {
            Q_OBJECT
        public:
            Extension();
             ~Extension() override;
            virtual QString GetVersion()=0;
            virtual QString GetName()=0;
            virtual QString GetDescription()=0;
            virtual QString GetAuthor()=0;
            virtual bool IsWorking()=0;
            virtual void Hook_Shutdown() {}
            virtual void Hook_OnScrollbackDestroyed(Scrollback *scrollback) {}
            virtual void Hook_OnNetworkDisconnect(IRCSession *session) {}
            //! When client receives unknown type of message from IRC server (most of these are sent to system window)
            virtual void Hook_OnNetworkUnknown(IRCSession *session, libircclient::Parser *px) {}
            virtual void Hook_OnNetworkGeneric(IRCSession *session, libircclient::Parser *px) {}

        //signals:

        public slots:
    };
}

#endif // EXTENSION_H
