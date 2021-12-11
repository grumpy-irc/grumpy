//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2021

#ifndef NETWORKWIN_H
#define NETWORKWIN_H

#include <QDialog>

namespace Ui {
    class NetworkWin;
}

namespace GrumpyIRC
{
    class NetworkWin : public QDialog
    {
            Q_OBJECT

        public:
            explicit NetworkWin(QWidget *parent = nullptr);
            ~NetworkWin();
            void Load(int network_id);

        private slots:
            void on_pushSave_clicked();
            void on_pushExit_clicked();

        private:
            QHash<int, int> identityMap;
            Ui::NetworkWin *ui;
            int networkID = -1;
    };
}

#endif // NETWORKWIN_H
