//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
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

namespace GrumpyIRC
{
    class InputBox;
    class ScrollbackFrame;
    class UserWidget;
    class ScrollbackList;
    class ScrollbacksManager;
    class Skin;
    class SyslogWindow;

    class MainWindow : public QMainWindow
    {
            Q_OBJECT
        public:
            static MainWindow *Main;

            explicit MainWindow(QWidget *parent = 0);
            MainWindow(bool fork, MainWindow *parent);
            ~MainWindow();
            ScrollbacksManager *GetScrollbackManager();
            //! Return a pointer to widget that contains list of all windows
            ScrollbackList *GetScrollbackList();
            void WriteToSystemWindow(QString text);
            void WriteToCurrentWindow(QString text);
            ScrollbackFrame *GetSystem();
            ScrollbackFrame *GetCurrentScrollbackFrame();
            UserWidget *GetUsers();
            void Fork();
            void SetWN(QString text);
            void UpdateStatus();
            void OpenGrumpy(QString hostname, int port, QString username, QString password, bool ssl);
            void OpenIRCNetworkLink(QString link);
            void OpenServer(libirc::ServerAddress server);

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

        private:
            void closeEvent(QCloseEvent *event);
            bool isFork;
            QTimer timer;
            QLabel *statusFrame;
            QLabel *identFrame;
            ScrollbackFrame *systemWindow;
            ScrollbackList *windowList;
            UserWidget *userWidget;
            SyslogWindow *syslogWindow;
            ScrollbacksManager *scrollbackWindow;
            Ui::MainWindow *ui;
    };
}

#endif // MAINWINDOW_H
