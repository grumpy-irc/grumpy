//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef ABOUTWIN_H
#define ABOUTWIN_H

#include "grumpy_global.h"
#include <libcore/definitions.h>
#include <QDialog>

namespace Ui
{
    class AboutWin;
}

namespace GrumpyIRC
{
    class LIBGRUMPYSHARED_EXPORT AboutWin : public QDialog
    {
            Q_OBJECT

        public:
            explicit AboutWin(QWidget *parent = nullptr);
            ~AboutWin();

        private:
            Ui::AboutWin *ui;
    };
}

#endif // ABOUTWIN_H
