//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef SCRIPTWIN_H
#define SCRIPTWIN_H

#include "grumpy_global.h"

#include <QDialog>

namespace Ui
{
    class ScriptWin;
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

    class LIBGRUMPYSHARED_EXPORT ScriptWin : public QDialog
    {
            Q_OBJECT

        public:
            explicit ScriptWin(ScrollbackFrame *parent = 0);
            ~ScriptWin();
            void Set(QString input);

        private slots:
            void on_pushButton_clicked();

        private:
            libircclient::Network *Network;
            ScrollbackFrame *parentFrame;
            Ui::ScriptWin *ui;
    };
}

#endif // SCRIPTWIN_H
