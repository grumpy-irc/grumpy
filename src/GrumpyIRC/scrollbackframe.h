//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SCROLLBACKFRAME_H
#define SCROLLBACKFRAME_H

#include <QFrame>
#include "../libcore/scrollback.h"

namespace Ui
{
    class ScrollbackFrame;
}

namespace GrumpyIRC
{
    class InputBox;
    class ScrollbackList_Window;

    class ScrollbackFrame : public QFrame, public Scrollback
    {
            Q_OBJECT

        public:
            explicit ScrollbackFrame(ScrollbackFrame *parentWindow = NULL, QWidget *parent = NULL);
            ~ScrollbackFrame();
            QString GetWindowName() const;
            void InsertText(ScrollbackItem item);
            void SetWindowName(QString title);
            void InsertChild(ScrollbackFrame *child);
            ScrollbackFrame *Child(int row);
            int ChildCount() const;
            int ModelRow() const;
            ScrollbackFrame *GetParent();
            void SetParent(ScrollbackFrame* parentWindow);
            bool IsDeletable;

        private:
            QString buffer;
            QString _name;
            InputBox *inputBox;
            Ui::ScrollbackFrame *ui;
            QList<ScrollbackFrame*> childItems;
            ScrollbackFrame *_parent;
    };
}

#endif // SCROLLBACKFRAME_H
