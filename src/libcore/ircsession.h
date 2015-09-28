//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef IRCSESSION_H
#define IRCSESSION_H

#include <QMutex>
#include <QString>
#include "libcore_global.h"
#include "../libirc/libircclient/user.h"
#include "../libirc/libircclient/channel.h"
#include "../libirc/libircclient/network.h"

namespace GrumpyIRC
{
    class Scrollback;
    class LIBCORESHARED_EXPORT IRCSession
    {
        public:
            static IRCSession *Open(Scrollback *system_window, QString hostname, QString network = "", QString nick = "", QString password = "", bool ssl = false);
            static QMutex Sessions_Lock;
			static QList<IRCSession*> Sessions;
			
            /*!
             * \brief IRCSession Creates a new uninitialized session, you should always create new sessions
             *                   with IRCSession::Open() instead of calling this directly
             */
            IRCSession(Scrollback *system);
            virtual ~IRCSession();
            virtual Scrollback *GetSystemWindow();
            virtual libircclient::Network *GetNetwork();
            virtual void Connect(libircclient::Network *Network);
            bool IsConnected();
        private:
            libircclient::Network *network;
            Scrollback *systemWindow;
    };
}

#endif // IRCSESSION_H
