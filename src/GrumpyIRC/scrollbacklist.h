//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SCROLLBACKLIST_H
#define SCROLLBACKLIST_H

#include "scrollbacklist_itemmodel.h"
#include <QDockWidget>

namespace Ui
{
    class ScrollbackList;
}

namespace GrumpyIRC
{
    class ScrollbackFrame;
    class ScrollbackList : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit ScrollbackList(QWidget *parent = 0);
            ~ScrollbackList();
            void RegisterWindow(ScrollbackFrame *scrollback, ScrollbackFrame *parentWindow = NULL);
            ScrollbackFrame *GetRootTreeItem();

        private slots:
            void on_treeView_activated(const QModelIndex &index);

        private:
            ScrollbackList_ItemModel *model;
            Ui::ScrollbackList *ui;
    };
}

#endif // SCROLLBACKLIST_H
