//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef USERWIDGET_H
#define USERWIDGET_H

#include "grumpy_global.h"
#include <QDockWidget>
#include <QFrame>

namespace Ui
{
    class UserWidget;
}

namespace GrumpyIRC
{
    class LIBGRUMPYSHARED_EXPORT UserWidget : public QDockWidget
    {
            Q_OBJECT

        public:
            explicit UserWidget(QWidget *parent = 0);
            ~UserWidget();
            void SetFrame(QFrame *frame);

        private:
            Ui::UserWidget *ui;
    };
}

#endif // USERWIDGET_H
