//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef GRUMPYDCFWIN_H
#define GRUMPYDCFWIN_H

#include "grumpy_global.h"
#include <QDialog>
#include <QHash>
#include <QTimer>
#include <QDateTime>

namespace Ui
{
    class GrumpydCfWin;
}

namespace GrumpyIRC
{
    class GrumpydSession;
    class LIBGRUMPYSHARED_EXPORT GrumpydCfWin : public QDialog
    {
            Q_OBJECT

        public:
            explicit GrumpydCfWin(GrumpydSession *session, QWidget *parent = nullptr);
            ~GrumpydCfWin() override;
            GrumpydSession *GrumpySession;

        private slots:
            void on_buttonBox_accepted();
            void on_tabWidget_currentChanged(int index);
            void OnRefresh();
            void on_tableUser_customContextMenuRequested(const QPoint &position);

        private:
            template <typename T>
            void set(QString key, T value);
            QString getString(QString key, QString missing);
            unsigned int getUInt(QString key, unsigned int default_uint);
            bool getBool(QString key, bool default_bool);
            void ClearUserList();
            void RefreshUserList();
            Ui::GrumpydCfWin *ui;
            QHash<QString, unsigned int> uid;
            //! Remember if we loaded user window, so that we don't need to load userlist everytime this form is open
            //! we only load it when user switch the tab there and when this variable is false
            bool userLoaded = false;
            QDateTime lastKnownRefresh;
            QTimer *timer;
    };
}

#endif // GRUMPYDCFWIN_H
