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

#include <QDockWidget>
#include <QStandardItemModel>

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
			void RegisterWindow(ScrollbackFrame *scrollback, QStandardItem *parent_node = NULL);
            QStandardItem *GetRootTreeItem();

        private slots:
            void on_treeView_activated(const QModelIndex &index);
            void on_treeView_customContextMenuRequested(const QPoint &pos);

            void on_treeView_clicked(const QModelIndex &index);

        private:
            void switchWindow(const QModelIndex &index);
			QStandardItem *root;
            QStandardItemModel *model;
            Ui::ScrollbackList *ui;
    };
}

#endif // SCROLLBACKLIST_H
