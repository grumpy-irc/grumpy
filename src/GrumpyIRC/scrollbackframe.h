//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SCROLLBACKFRAME_H
#define SCROLLBACKFRAME_H

#include <QFrame>
#include <QStandardItemModel>
#include "stextbox.h"
#include "../libirc2htmlcode/parser.h"
#include "../libcore/scrollback.h"

namespace Ui
{
    class ScrollbackFrame;
}

namespace libircclient
{
    class Network;
    class User;
}

namespace GrumpyIRC
{
    class InputBox;
    class Scrollback;
    class ScrollbackList_Node;
    class ScrollbackList_Window;
    class UserFrame;
    class NetworkSession;

    class ScrollbackFrame : public QFrame
    {
            Q_OBJECT

        public:
            static irc2htmlcode::Parser parser;

            explicit ScrollbackFrame(ScrollbackFrame *parentWindow = NULL, QWidget *parent = NULL, Scrollback *_scrollback = NULL);
            ~ScrollbackFrame();
            QString GetWindowName() const;
            void InsertText(QString text);
            void InsertText(ScrollbackItem item);
            void SetWindowName(QString title);
			ScrollbackFrame *GetParent();
            unsigned long GetID();
            NetworkSession *GetSession();
            Scrollback *GetScrollback();
            UserFrame *GetUserFrame();
            void Focus();
            bool IsChannel();
            bool IsNetwork();
            bool IsDead();
            void RequestClose();
            void UpdateIcon();
            void RequestPart();
            void RequestDisconnect();
            void RequestMore(int count);
            void RefreshHtml();
            void RefreshHtmlIfNeeded();
            scrollback_id_t GetItems();
            int GetSynced();
            //void SetParent(ScrollbackFrame* parentWindow);
            bool IsDeletable;
            bool IsVisible;
            ScrollbackList_Node *TreeNode;
        private slots:
            void _insertText_(ScrollbackItem item);
            void UserList_Insert(libircclient::User *ux);
            void UserList_Remove(QString user);
            void UserList_Rename(QString old, libircclient::User *us);
            void OnDead();
            void Refresh();
            void Menu(QPoint pn);
            void OnClosed();
            void NetworkChanged(libircclient::Network *network);
        private:
            void clearItems();
            void writeText(ScrollbackItem item);
            bool isClean;
            int maxItems;
            QList<ScrollbackItem> unwritten;
            STextBox *textEdit;
            bool needsRefresh;
            Scrollback *scrollback;
			//QStandardItem *treeNode;
            QString buffer;
            UserFrame *userFrame;
            QString _name;
            InputBox *inputBox;
            Ui::ScrollbackFrame *ui;
            ScrollbackFrame *_parent;
    };
}

#endif // SCROLLBACKFRAME_H
