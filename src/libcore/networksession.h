//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef NETWORKSESSION_H
#define NETWORKSESSION_H

#include <QString>
#include <QDateTime>
#include "../libirc/libircclient/priority.h"
#include <QObject>

namespace libircclient
{
    class User;
    class Network;
    class Channel;
}

namespace GrumpyIRC
{
    enum SessionType
    {
        SessionType_IRC,
        SessionType_Grumpyd
    };

    class Scrollback;

    class NetworkSession : public QObject
    {
            Q_OBJECT
        public:
            NetworkSession();
            virtual ~NetworkSession();
            virtual bool IsAutoreconnect(Scrollback *window)=0;
            virtual void SetAutoreconnect(Scrollback *window, bool reconnect)=0;
            virtual bool IsAway(Scrollback *scrollback = NULL)=0;
            virtual bool IsConnected() const=0;
            virtual libircclient::Network *GetNetwork(Scrollback *window = 0)=0;
            virtual void SendAction(Scrollback *window, QString text)=0;
            virtual void SendMessage(Scrollback *window, QString text)=0;
            virtual void SendRaw(Scrollback *window, QString raw, libircclient::Priority pr = libircclient::Priority_Normal)=0;
            virtual void SendCTCP(Scrollback *window, QString target, QString ctcp, QString param)=0;
            virtual void SendNotice(Scrollback *window, QString text)=0;
            virtual QList<QString> GetChannels(Scrollback *window)=0;
            virtual libircclient::User *GetSelfNetworkID(Scrollback *window)=0;
            virtual QString GetLocalUserModeAsString(Scrollback *window)=0;
            virtual void SendMessage(Scrollback *window, QString target, QString message)=0;
            virtual void SendNotice(Scrollback *window, QString target, QString message)=0;
            virtual void Query(Scrollback *window, QString target, QString message)=0;
            virtual Scrollback *GetSystemWindow()=0;
            virtual libircclient::Channel *GetChannel(Scrollback *window)=0;
            virtual void RetrieveChannelBanList(Scrollback *window, QString channel_name)=0;
            virtual void RequestReconnect(Scrollback *window)=0;
            virtual void RequestDisconnect(Scrollback *window, QString reason, bool auto_delete)=0;
            virtual void SetAway(QString reason);
            virtual void UnsetAway();
            //! Request the selected window to be removed from window tree
            //! the windows are never directly removed because there might be complex structures depending on them
            //! you always need to ASK the window to delete itself
            virtual void RequestRemove(Scrollback *window)=0;
            virtual void RequestPart(Scrollback *window)=0;
            virtual SessionType GetType()=0;
            // Workaround for really weird MSVC bug in their compiler
#ifdef _MSC_VER
            QDateTime GetCreationDateTime();
            QDateTime GetConnectionDateTime();
            bool IsAway();
#else
            virtual QDateTime GetCreationDateTime();
            virtual QDateTime GetConnectionDateTime();
            virtual bool IsAway();
#endif
        protected:
            QDateTime createdOn;
            QDateTime connectedOn;
            bool isAway;
        signals:
            void Event_Deleted();
            //! Used when authentication to underlying protocol fails
            void Event_AuthenticationFailed();

        //public slots:
    };
}

#endif // NETWORKSESSION_H
