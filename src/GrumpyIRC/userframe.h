//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef USERFRAME_H
#define USERFRAME_H

#include <QFrame>
#include <QHash>
#include <QListWidgetItem>
#include "userframeitem.h"
#include "../libirc/libircclient/user.h"

namespace libircclient { class Network; }

namespace Ui
{
    class UserFrame;
}

namespace GrumpyIRC
{
    class UserFrame : public QFrame
    {
            Q_OBJECT

        public:
            explicit UserFrame(QWidget *parent = 0);
            ~UserFrame();
            void InsertUser(libircclient::User *user);
            void RemoveUser(QString user);
            void ChangeNick(QString new_nick, QString old_nick);
            void SetNetwork(libircclient::Network *Network);
            void UpdateInfo();

        private slots:
            void on_listWidget_doubleClicked(const QModelIndex &index);
            void on_listWidget_clicked(const QModelIndex &index);
            void on_listWidget_customContextMenuRequested(const QPoint &pos);

        private:
            libircclient::Network *network;
            QHash<QString, libircclient::User> users;
            QHash<QString, QListWidgetItem*> userItem;
            Ui::UserFrame *ui;
    };
}

#endif // USERFRAME_H
