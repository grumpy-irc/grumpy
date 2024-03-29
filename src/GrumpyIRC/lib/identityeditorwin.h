//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2021

#ifndef IDENTITYEDITORWIN_H
#define IDENTITYEDITORWIN_H

#include <QDialog>
#include "grumpy_global.h"

namespace Ui
{
    class IdentityEditorWin;
}

namespace GrumpyIRC
{
    class LIBGRUMPYSHARED_EXPORT IdentityEditorWin : public QDialog
    {
            Q_OBJECT
        public:
            explicit IdentityEditorWin(QWidget *parent = nullptr);
            ~IdentityEditorWin();
            void Load(int id);

        private slots:
            void on_pbSave_clicked();

            void on_pbCancel_clicked();

        private:
            int identID = -1;
            Ui::IdentityEditorWin *ui;
    };
}

#endif // IDENTITYEDITORWIN_H
