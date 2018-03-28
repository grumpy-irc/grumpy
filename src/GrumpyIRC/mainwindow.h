//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../libcore/scrollback.h"
#include <QLabel>
#include <QSystemTrayIcon>
#include <QMainWindow>
#include <QTimer>

namespace Ui
{
    class MainWindow;
}

namespace libirc
{
    class ServerAddress;
}

class QProgressBar;

namespace GrumpyIRC
{
    class InputBox;
    class ScrollbackFrame;
    class UserWidget;
    class LinkHandler;
    class ScrollbackList;
    class ScrollbacksManager;
    class Skin;

    class MainWindow : public QMainWindow
    {
            Q_OBJECT
        public:
            static void Exit();
            static MainWindow *Main;

            explicit MainWindow(QWidget *parent = 0);
            MainWindow(bool fork, MainWindow *parent);
            ~MainWindow();
            ScrollbacksManager *GetScrollbackManager();
            //! Return a pointer to widget that contains list of all windows
            ScrollbackList *GetScrollbackList();
            void WriteToSystemWindow(QString text);
            void WriteToCurrentWindow(QString text, ScrollbackItemType item = ScrollbackItemType_System);
            ScrollbackFrame *GetSystem();
            ScrollbackFrame *GetCurrentScrollbackFrame();
            UserWidget *GetUsers();
            void Fork();
            void SetWN(QString text);
            void UpdateStatus();
            void Notify(QString heading, QString text);
            void OpenUrl(QString url);
            void UpdateSkin();
            void Execute(QString text);
            void HideProgress();
            void ShowProgress();
            void SetMaxProgressValue(int max);
            void SetProgress(int progress);
            void ExecuteLine(QString line);
            void OpenGrumpy(QString hostname, int port, QString username, QString password, bool ssl);
            void OpenIRCNetworkLink(QString link);
            void OpenServer(libirc::ServerAddress server);
            void EnableGrumpydContext(bool enable);
            void SetupAutoAway();
            void ResetAutoAway();

        private slots:
            void OnRefresh();
            void on_actionExit_triggered();
            void on_actionConnect_triggered();
            void on_actionAbout_triggered();
            void on_actionLoad_more_items_from_remote_triggered();
            void on_actionPreferences_triggered();
            void on_actionOpen_window_triggered();
            void on_actionFavorites_triggered();
            void on_actionToggle_secret_triggered();
            void on_actionProxy_triggered();
            void on_actionEnable_proxy_toggled(bool arg1);
            void OnAutoAway();
            void on_actionExport_to_html_triggered();
            void on_actionExport_to_plain_text_triggered();

        private:
            QString processInput(QString text);
            void closeEvent(QCloseEvent *event);
            QSystemTrayIcon tray;
            bool isFork;
            bool isAway;
            QTimer timer;
            QTimer *timerAway;
            QLabel *statusFrame;
            QLabel *identFrame;
            QLabel *windowCount;
            QLabel *overviewFrame;
            QProgressBar *progressBar;
            ScrollbackFrame *systemWindow;
            LinkHandler *handler;
            ScrollbackList *windowList;
            UserWidget *userWidget;
            ScrollbacksManager *scrollbackWindow;
            Ui::MainWindow *ui;
    };
}

#endif // MAINWINDOW_H
