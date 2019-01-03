//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef USERFRAME_H
#define USERFRAME_H

#include "grumpy_global.h"
#include <QFrame>
#include <QHash>
#include <QListWidgetItem>
#include "userframeitem.h"
#include <libcore/grumpyobject.h>
#include <libirc/libircclient/user.h>

namespace libircclient { class Network; }

namespace Ui
{
    class UserFrame;
}

namespace GrumpyIRC
{
    class ScrollbackFrame;

    class LIBGRUMPYSHARED_EXPORT UserFrame : public QFrame, public GrumpyObject
    {
            Q_OBJECT
        public:
            explicit UserFrame(ScrollbackFrame *parent);
            ~UserFrame() override;
            void InsertUser(libircclient::User *user, bool bulk);
            void RemoveUser(QString user);
            void RefreshUser(libircclient::User *user);
            void ChangeNick(const QString &new_nick, QString old_nick);
            void SetNetwork(libircclient::Network *Network);
            QList<libircclient::User> SelectedUsers();
            void Sort();
            QList<QString> GetUsers();
            void Whois();
            void ChangeMode(const QString &mode);
            void UpdateInfo();
            bool IsVisible;
            bool NeedsUpdate;

        private slots:
            void on_listWidget_doubleClicked(const QModelIndex &index);
            void on_listWidget_clicked(const QModelIndex &index);
            void on_listWidget_customContextMenuRequested(const QPoint &pos);

        private:
            QString generateKick();
            QString generateBan();
            void kick();
            void query();
            void changeModes(char prefix, char mode);
            void ban();
            void kb();
            void ctcp(const QString &text);
            QString GenerateTip(libircclient::User *ux);
            QHash<char, unsigned int> userCounts;
            ScrollbackFrame *parentFrame;
            libircclient::Network *network;
            QHash<QString, libircclient::User> users;
            QHash<QString, QListWidgetItem*> userItem;
            Ui::UserFrame *ui;
    };
}

#endif // USERFRAME_H
