//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef INITIALIZEWIN_H
#define INITIALIZEWIN_H

#include <QDialog>

namespace Ui
{
    class InitializeWin;
}

namespace GrumpyIRC
{
    class InitializeWin : public QDialog
    {
            Q_OBJECT

        public:
            explicit InitializeWin(QWidget *parent = 0);
            ~InitializeWin();
            void Process();

        private:
            Ui::InitializeWin *ui;
    };
}

#endif // INITIALIZEWIN_H
