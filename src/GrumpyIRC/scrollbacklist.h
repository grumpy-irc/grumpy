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

class QTimer;

namespace Ui
{
    class ScrollbackList;
}

namespace GrumpyIRC
{
    class ScrollbackList_Node;
    class ScrollbackFrame;
    class ScrollbackList : public QDockWidget
    {
            Q_OBJECT
        public:
            static ScrollbackList *GetScrollbackList();

            explicit ScrollbackList(QWidget *parent = 0);
            ~ScrollbackList();
            void RegisterHidden();
            void RegisterWindow(ScrollbackFrame *scrollback, ScrollbackList_Node *parent_node = NULL);
            QStandardItem *GetRootTreeItem();
            void UnregisterHidden();
            void UnregisterWindow(ScrollbackList_Node *node, ScrollbackList_Node *parent_n);
            bool ShowHidden = false;

        private slots:
            void OnUpdate();
            void on_treeView_activated(const QModelIndex &index);
            void on_treeView_customContextMenuRequested(const QPoint &pos);
            void on_treeView_clicked(const QModelIndex &index);

        private:
            static ScrollbackList *scrollbackList;

            void switchWindow(const QModelIndex &index);
            void sniffer(ScrollbackFrame *window);
            void closeWindow();
            ScrollbackFrame *selectedWindow();
            QStandardItem *root;
            QTimer *timer;
            QStandardItemModel *model;
            Ui::ScrollbackList *ui;
    };
}

#endif // SCROLLBACKLIST_H
