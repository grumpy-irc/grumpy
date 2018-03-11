//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef INPUTBOX_H
#define INPUTBOX_H

#include <QFrame>

namespace Ui
{
    class InputBox;
}

namespace GrumpyIRC
{
    class AutocompletionEngine;
    class ScrollbackFrame;
    class InputBox : public QFrame
    {
            Q_OBJECT

        public:
            static AutocompletionEngine *AE;

            explicit InputBox(ScrollbackFrame *parent = 0);
            ~InputBox();
            void ProcessInput();
            void Secure();
            void Complete();
            void Focus();
            void InsertAtCurrentPosition(QString text);
            void InsertEnter();
            bool IsSecure();
            void History(bool up = false);

        private slots:
            void on_lineEdit_returnPressed();

        private:
            void insertToHistory();
            bool isSecure;
            int historyPosition;
            unsigned int historySize;
            QStringList history;
            ScrollbackFrame *parent;
            Ui::InputBox *ui;
    };
}

#endif // INPUTBOX_H
