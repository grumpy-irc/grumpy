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
#include <QThread>
#include <QMutex>
#include <QTimer>
#include "stextbox.h"
#include "../libirc2htmlcode/parser.h"
#include "../libcore/scrollback.h"

#define GRUMPY_H_UNKNOWN 0
#define GRUMPY_H_YES     1
#define GRUMPY_H_NOT     2

#define GRUMPY_SCROLLER_TIME_WAIT 200

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
    class ScrollbackFrame;
    class NetworkSession;

    /*!
     * \brief The ScrollbackFrame_WorkerThread class processes CPU intensive items on background in order to speed up grumpy
     */
    class ScrollbackFrame_WorkerThread : public QThread
    {
            Q_OBJECT
        public:
            ScrollbackFrame_WorkerThread();
            void Sleep(int msec);
            bool IsRunning;
            bool IsFinished;
        protected:
            void run();
    };

    class ScrollbackFrame : public QFrame
    {
            Q_OBJECT

        public:
            static void UpdateSkins();
            static void ExitThread();
            static void InitializeThread();
            static QList<ScrollbackFrame*> ScrollbackFrames;
            static QMutex ScrollbackFrames_m;
            static irc2htmlcode::Parser parser;

            explicit ScrollbackFrame(ScrollbackFrame *parentWindow = NULL, QWidget *parent = NULL, Scrollback *_scrollback = NULL, bool is_system = false);
            ~ScrollbackFrame();
            QString GetWindowName() const;
            void InsertText(QString text, ScrollbackItemType item = ScrollbackItemType_System);
            void InsertText(ScrollbackItem item);
            void SetWindowName(QString title);
            bool IsConnectedToIRC();
            bool IsChannel();
            bool IsNetwork();
            bool IsGrumpy();
            bool IsHidden();
            void ToggleHide();
            bool IsVisible();
            bool IsDead();
            ScrollbackFrame *GetParent();
            scrollback_id_t GetID();
            NetworkSession *GetSession();
            Scrollback *GetScrollback();
            UserFrame *GetUserFrame();
            QString GetTitle();
            void UpdateColor();
            void Focus();
            void RequestClose();
            void UpdateIcon();
            void EnableState(bool enable);
            void RequestPart();
            void ToggleSecure();
            void ExecuteScript(QString text);
            void RequestJoin();
            void Reconnect();
            void RequestDisconnect();
            void RequestMore(unsigned int count);
            void RefreshHtml();
            void SendCtcp(QString target, QString ctcp, QString text);
            void RefreshHtmlIfNeeded();
            void SetProperty(QString name, QVariant value);
            libircclient::Network *GetNetwork();
            void TransferRaw(QString data);
            libircclient::User *GetIdentity();
            scrollback_id_t GetItems();
            QList<QString> GetUsers();
            QList<QString> GetChannels();
            QString GetLocalUserMode();
            void UpdateSkin();
            int GetSynced();
            //void SetParent(ScrollbackFrame* parentWindow);
            bool IsDeletable;
            bool ShowJQP;
            bool Highlighting;
            void SetVisible(bool is_visible);
            ScrollbackList_Node *TreeNode;
            bool Refreshing = false;
            QDateTime LastMenuTooltipUpdate;
            bool IsSystem;
        protected:
            static ScrollbackFrame_WorkerThread *WorkerThread;
        private slots:
            void _insertText_(ScrollbackItem item);
            void UserList_Insert(libircclient::User *ux, bool bulk);
            void UserList_Refresh(libircclient::User *ux);
            void OnState();
            void UserList_Remove(QString user, bool bulk);
            void UserList_Rename(QString old, libircclient::User *us);
            void OnFinishSortBulk();
            void OnDead();
            void OnLink(QString url);
            void OnScroll();
            void Refresh();
            void Menu(QPoint pn);
            void OnClosed();
            void NetworkChanged(libircclient::Network *network);
        private:
            friend class ScrollbackFrame_WorkerThread;
            void clearItems();
            void writeText(ScrollbackItem item, int highlighted = 0);
            QString itemsToString(QList<ScrollbackItem> items);
            bool isVisible;
            QTimer scroller;
            bool isClean;
            int maxItems;
            int currentScrollbar;
            QString unwrittenBlock;
            QList<ScrollbackItem> unwritten;
            QMutex unwritten_m;
            libircclient::Network *precachedNetwork;
            STextBox *textEdit;
            bool needsRefresh;
            Scrollback *scrollback;
            QString buffer;
            UserFrame *userFrame;
            QString _name;
            InputBox *inputBox;
            Ui::ScrollbackFrame *ui;
            ScrollbackFrame *_parent;
    };
}

#endif // SCROLLBACKFRAME_H
