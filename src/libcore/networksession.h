//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef NETWORKSESSION_H
#define NETWORKSESSION_H

#include <QString>

namespace libircclient
{
    class User;
    class Network;
}

namespace GrumpyIRC
{
    enum SessionType
    {
        SessionType_IRC,
        SessionType_Grumpyd
    };

    class Scrollback;

    class NetworkSession
    {
        public:
            NetworkSession();
            virtual ~NetworkSession();
            virtual bool IsConnected() const=0;
            virtual libircclient::Network *GetNetwork()=0;
            virtual void SendAction(Scrollback *window, QString text)=0;
            virtual void SendMessage(Scrollback *window, QString text)=0;
            virtual void SendRaw(Scrollback *window, QString raw)=0;
            virtual QList<QString> GetChannels(Scrollback *window)=0;
            virtual libircclient::User *GetSelfNetworkID(Scrollback *window)=0;
            virtual Scrollback *GetSystemWindow()=0;
            virtual void RequestDisconnect(Scrollback *window, QString reason, bool auto_delete) = 0;
            //! Request the selected window to be removed from window tree
            //! the windows are never directly removed because there might be complex structures depending on them
            //! you always need to ASK the window to delete itself
            virtual void RequestRemove(Scrollback *window)=0;
            virtual void RequestPart(Scrollback *window)=0;
            virtual SessionType GetType()=0;

        //signals:

        //public slots:
    };
}

#endif // NETWORKSESSION_H
