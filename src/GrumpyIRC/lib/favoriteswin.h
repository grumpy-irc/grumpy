//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2021

#ifndef FAVORITESWIN_H
#define FAVORITESWIN_H

#include "grumpy_global.h"
#include <QDialog>

namespace Ui
{
    class FavoritesWin;
}

namespace GrumpyIRC
{
    class LIBGRUMPYSHARED_EXPORT FavoritesWin : public QDialog
    {
            Q_OBJECT

        public:
            explicit FavoritesWin(QWidget *parent = 0);
            ~FavoritesWin();
            void RefreshNetworks();
            void RefreshIdentities();

        private slots:
            void on_tv_IdentityList_customContextMenuRequested(const QPoint &pos);
            void on_tv_NetworkList_customContextMenuRequested(const QPoint &pos);

        private:
            QList<int> selectedNetworks();
            QList<int> selectedIdents();
            Ui::FavoritesWin *ui;
    };
}

#endif // FAVORITESWIN_H
