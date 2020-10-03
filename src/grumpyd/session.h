//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef SESSION_H
#define SESSION_H

#include "../libcore/definitions.h"
#include <QObject>
#include <QHash>
#include <QThread>
#include <QList>
#include <QTcpSocket>
#include <QString>
#include <QMutex>
#include "../libgp/gp.h"

namespace GrumpyIRC
{
    class Scrollback;
    class User;

    class Session : public QThread
    {
            enum State
            {
                State_Login,
                State_Open,
                State_Killing,
                State_Exiting,
                State_Offline
            };

            Q_OBJECT
        public:
            static QList<Session*> Sessions();

            Session(qintptr socket_ptr, bool ssl);
            ~Session() override;
            void run() override;
            unsigned long GetSID();
            bool IsAuthorized(const QString &permission);
            //! Transfer data to every session this user has, so that every session connected as this user receives it
            void SendToEverySession(gp_command_t command, const QHash<QString, QVariant>& parameters);
            void SendToOtherSessions(gp_command_t command, const QHash<QString, QVariant>& parameters);
            Scrollback *GetScrollback(scrollback_id_t scrollback_id);
            void TransferError(gp_command_t source, const QString& description, int id);
            void PermissionDeny(gp_command_t source);
            void Kick();
            void Shutdown();
            void Disconnect();
            bool IsRunning;
            bool IsAway;
            QString AwayReason;
            int MaxScrollbackSyncItems;
            State SessionState;

        private slots:
            void OnDisconnected();
            void OnCommand(gp_command_t text, QHash<QString, QVariant> parameters);
            void OnGPError(QString text, int code);

        signals:
            void OnError(int error, QString text);
            void OnAway();

        private:
            static unsigned long lSID;
            static QList<Session*> SessionList;
            static QMutex *sessions_lock;

            void processSniffer(QHash<QString, QVariant> parameters);
            void processLogin(QHash<QString, QVariant> parameters);
            void processNetworks();
            void processIrcQuit(QHash<QString, QVariant> parameters);
            void processRequest(QHash<QString, QVariant> parameters);
            void processSetup(QHash<QString, QVariant> parameters);
            void processResync(QHash<QString, QVariant> parameters);
            void processReconnect(QHash<QString, QVariant> parameters);
            void processOptions(QHash<QString, QVariant> parameters);
            void processRemove(QHash<QString, QVariant> parameters);
            void processPBResync(QHash<QString, QVariant> parameters);
            void processHideSB(QHash<QString, QVariant> parameters);
            void processMessage(QHash<QString, QVariant> parameters);
            void processCommand(QHash<QString, QVariant> parameters);
            //! Called when user wants to connect to new IRC server
            void processNew(QHash<QString, QVariant> info);
            void processInfo(QHash<QString, QVariant> parameters);
            void processQuery(QHash<QString, QVariant> parameters);
            //! Called when admin wants to list users of grumpyd
            void processUserList(QHash<QString, QVariant> parameters);
            void processLockUser(QHash<QString, QVariant> parameters);
            void processUnlockUser(QHash<QString, QVariant> parameters);
            void processCreateUser(QHash<QString, QVariant> parameters);
            void processRemoveUser(QHash<QString, QVariant> parameters);
            void processStorageSet(QHash<QString, QVariant> parameters);
            void processStorageDel(QHash<QString, QVariant> parameters);
            void processStorageGet(QHash<QString, QVariant> parameters);
            void processAway(QHash<QString, QVariant> parameters);
            void processInstallScript(QHash<QString, QVariant> parameters);
            void processRemoveScript(QHash<QString, QVariant> parameters);
            void processScriptLS(QHash<QString, QVariant> parameters);
            void processScriptReadSource(QHash<QString, QVariant> parameters);

            QTcpSocket *socket;
            QString peer;
            bool usingSsl;
            User *loggedUser;
            libgp::GP *protocol;
            unsigned long SID;
    };
}

#endif // SESSION_H
