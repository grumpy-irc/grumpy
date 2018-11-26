//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef STEXTBOX_H
#define STEXTBOX_H

#include "grumpy_global.h"
#include <libcore/grumpyobject.h>
#include <QPlainTextEdit>

namespace GrumpyIRC
{
    class LIBGRUMPYSHARED_EXPORT STextBox : public QPlainTextEdit, public GrumpyObject
    {
            Q_OBJECT
        public:
            STextBox(QWidget *parent = NULL);
            ~STextBox();
            void AppendHtml(QString html);
            void SetStyleSheet(QString css);
            void Clear();
            bool SendEvent(QEvent *e);
        signals:
            void Event_Link(QString text);
        protected:
            void dropEvent(QDropEvent *e);
            void scrollContentsBy();
            void mousePressEvent(QMouseEvent *e);
            void mouseReleaseEvent(QMouseEvent *e);
            QString clickedAnchor;
    };
}

#endif // STEXTBOX_H
