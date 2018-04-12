//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef PACKETSNIFFERWIN_H
#define PACKETSNIFFERWIN_H

#include "grumpy_global.h"
#include <QDialog>

namespace Ui
{
    class PacketSnifferWin;
}

class QTimer;

namespace GrumpyIRC
{
    class IRCSession;
    class GrumpydSession;
    class LIBGRUMPYSHARED_EXPORT PacketSnifferWin : public QDialog
    {
            Q_OBJECT

        public:
            explicit PacketSnifferWin(QWidget *parent = 0);
            ~PacketSnifferWin();
            void Load(IRCSession *session);
            void Load(GrumpydSession *session, IRCSession *network);

        private slots:
            void OnRefresh();

        private:
            QTimer *timer;
            Ui::PacketSnifferWin *ui;
            GrumpydSession *grumpyd = nullptr;
            IRCSession *irc = nullptr;
    };
}

#endif // PACKETSNIFFERWIN_H
