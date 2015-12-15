//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef CHANNELWIN_H
#define CHANNELWIN_H

#include "../libcore/definitions.h"

#include <QList>
#include <QCheckBox>
#include <QDialog>

namespace Ui
{
    class ChannelWin;
}

namespace libircclient
{
    class User;
    class Mode;
    class Channel;
    class Network;
}

namespace GrumpyIRC
{
    class ScrollbackFrame;
    class NetworkSession;

    class ChannelWin : public QDialog
    {
            Q_OBJECT

        public:
            explicit ChannelWin(NetworkSession *session, libircclient::Network *network, libircclient::Channel *channel, ScrollbackFrame *parent);
            ~ChannelWin();

        private slots:
            void on_pushButton_clicked();
            void on_plainTextEdit_textChanged();
            void OnMode(bool toggled);
            void on_tableWidget_2_customContextMenuRequested(const QPoint &pos);
            void on_tableWidget_3_customContextMenuRequested(const QPoint &pos);
            void on_tableWidget_4_customContextMenuRequested(const QPoint &pos);

        private:
            void headings();
            QList<int> selected_bx();
            void topic();
            ScrollbackFrame *parentFrame;
            libircclient::Mode *chanmode;
            bool ignore;
            bool updateTopic;
            QList<QCheckBox*> checkBoxesMode;
            NetworkSession *_ns;
            libircclient::Network *_network;
            libircclient::Channel *_channel;
            Ui::ChannelWin *ui;
    };
}

#endif // CHANNELWIN_H
