//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef SCRIPTINGMANAGER_HPP
#define SCRIPTINGMANAGER_HPP

#include "../grumpy_global.h"
#include <QDialog>

namespace Ui
{
    class ScriptingManager;
}

namespace GrumpyIRC
{
    class GrumpydSession;

    class ScriptingManager : public QDialog
    {
            Q_OBJECT
        public:
            explicit ScriptingManager(QWidget *parent = nullptr, GrumpydSession *remote = nullptr);
            ~ScriptingManager();
            void Reload();
            void LoadFile(QString path);

        private slots:
            void on_bLoad_clicked();
            void on_bReload_clicked();
            void on_tableWidget_customContextMenuRequested(const QPoint &pos);
            void on_pushScript_clicked();

        private:
            void unloadSelectSc();
            void deleteSelectSc();
            void reloadSelectSc();
            QList<int> selectedRows();
            GrumpydSession *remoteSession;
            Ui::ScriptingManager *ui;
    };
}

#endif // SCRIPTINGMANAGER_HPP
