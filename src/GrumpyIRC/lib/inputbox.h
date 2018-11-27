//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef INPUTBOX_H
#define INPUTBOX_H

#include "grumpy_global.h"
#include <libcore/grumpyobject.h>
#include <QFrame>
#include <QList>
#include <QString>

namespace Ui
{
    class InputBox;
}

namespace GrumpyIRC
{
    class AutocompletionEngine;
    class ScrollbackFrame;
    class LIBGRUMPYSHARED_EXPORT InputBox : public QFrame, public GrumpyObject
    {
            Q_OBJECT

        public:
            static AutocompletionEngine *AE;

            explicit InputBox(ScrollbackFrame *parent = nullptr);
            ~InputBox();
            void ProcessInput();
            void Secure();
            void Complete();
            void ClearHistory();
            //! Load data into history
            void LoadHistory(QList<QString> new_history);
            ScrollbackFrame *GetParent();
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
