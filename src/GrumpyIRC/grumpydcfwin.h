//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef GRUMPYDCFWIN_H
#define GRUMPYDCFWIN_H

#include <QDialog>

namespace Ui
{
    class GrumpydCfWin;
}

namespace GrumpyIRC
{
    class GrumpydSession;

    class GrumpydCfWin : public QDialog
    {
            Q_OBJECT

        public:
            explicit GrumpydCfWin(QWidget *parent = 0);
            ~GrumpydCfWin();
            GrumpydSession *GrumpySession;

        private slots:
            void on_buttonBox_accepted();

        private:
            Ui::GrumpydCfWin *ui;
    };
}

#endif // GRUMPYDCFWIN_H