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
            virtual void SendMessage(Scrollback *window, QString text)=0;
            virtual SessionType GetType()=0;

        //signals:

        //public slots:
    };
}

#endif // NETWORKSESSION_H
