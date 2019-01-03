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
             ~NetworkSession() override;
            virtual bool IsAutoreconnect(Scrollback *scrollback)=0;
            virtual void SetAutoreconnect(Scrollback *scrollback, bool reconnect)=0;
            virtual bool IsAway(Scrollback *scrollback = nullptr)=0;
            virtual bool IsConnected() const=0;
            virtual libircclient::Network *GetNetwork(Scrollback *scrollback = nullptr)=0;
            virtual void SendAction(Scrollback *scrollback, QString text)=0;
            virtual void SendMessage(Scrollback *scrollback, QString text)=0;
            virtual void SendRaw(Scrollback *scrollback, QString raw, libircclient::Priority pr = libircclient::Priority_Normal)=0;
            virtual void SendCTCP(Scrollback *scrollback, QString target, QString ctcp, QString param)=0;
            virtual void SendNotice(Scrollback *scrollback, QString text)=0;
            virtual QList<QString> GetChannels(Scrollback *scrollback)=0;
            virtual libircclient::User *GetSelfNetworkID(Scrollback *scrollback)=0;
            virtual QString GetLocalUserModeAsString(Scrollback *scrollback)=0;
            virtual void SendMessage(Scrollback *scrollback, QString target, QString message)=0;
            virtual void SendNotice(Scrollback *scrollback, QString target, QString message)=0;
            virtual void Query(Scrollback *scrollback, QString target, QString message)=0;
            virtual Scrollback *GetSystemWindow()=0;
            virtual libircclient::Channel *GetChannel(Scrollback *scrollback)=0;
            virtual void RetrieveChannelBanList(Scrollback *scrollback, QString channel_name)=0;
            virtual void RequestReconnect(Scrollback *scrollback)=0;
            virtual void RequestDisconnect(Scrollback *scrollback, QString reason, bool auto_delete)=0;
            virtual void SetAway(QString reason);
            virtual void UnsetAway();
            //! Request the selected scrollback to be removed from scrollback tree
            //! the scrollbacks are never directly removed because there might be complex structures depending on them
            //! you always need to ASK the scrollback to delete itself
            virtual void RequestRemove(Scrollback *scrollback)=0;
            virtual void RequestPart(Scrollback *scrollback)=0;
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
