//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef GRUMPYDUSERCFGWIN_H
#define GRUMPYDUSERCFGWIN_H

#include "grumpy_global.h"
#include <QDialog>
#include <libcore/definitions.h>

namespace Ui
{
    class GrumpydUserCfgWin;
}

namespace GrumpyIRC
{
    class GrumpydSession;

    class LIBGRUMPYSHARED_EXPORT GrumpydUserCfgWin : public QDialog
    {
        Q_OBJECT

        public:
            explicit GrumpydUserCfgWin(GrumpydSession *session, QWidget *parent = 0);
            ~GrumpydUserCfgWin();
            void UpdateUser(user_id_t id, QString name, QString role);

        private slots:
            void on_Update_clicked();

        private:
            void Error(QString what);
            QString original_role;
            int user_id = -1;
            GrumpydSession *Session;
            Ui::GrumpydUserCfgWin *ui;
    };
}

#endif // GRUMPYDUSERCFGWIN_H
