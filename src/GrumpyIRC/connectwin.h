//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef CONNECTWIN_H
#define CONNECTWIN_H

#include <QDialog>

namespace Ui
{
    class ConnectWin;
}

namespace GrumpyIRC
{
    class ConnectWin : public QDialog
    {
            Q_OBJECT

        public:
            explicit ConnectWin(QWidget *parent = 0);
            ~ConnectWin();

        private slots:
            void on_pushButton_clicked();
            void on_comboBox_currentIndexChanged(int index);
            void on_checkBox_toggled(bool checked);

        private:
            Ui::ConnectWin *ui;
    };
}

#endif // CONNECTWIN_H
