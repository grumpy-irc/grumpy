//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#ifdef MessageBox
#undef MessageBox
#endif

#include "../libcore/definitions.h"
#include <QDialog>

namespace Ui
{
    class MessageBox;
}

namespace GrumpyIRC
{
    enum MessageBoxType
    {
        MessageBoxType_Normal,
        MessageBoxType_Error,
        MessageBoxType_Question,
        MessageBoxType_QuestionCancel
    };

    enum MessageBoxResponse
    {
        MessageBoxResponse_OK,
        MessageBoxResponse_Yes,
        MessageBoxResponse_No,
        MessageBoxResponse_Cancel
    };

    class MessageBox : public QDialog
    {
            Q_OBJECT

        public:
            static void Display(QString id, QString title, QString message, QWidget *parent = 0);

            explicit MessageBox(QString id, QString title, QString message, QWidget *parent = 0);
            MessageBoxResponse Exec(MessageBoxType type);
            ~MessageBox();

        private slots:
            void on_pushYES_clicked();

            void on_pushNO_clicked();

            void on_pushCancel_clicked();

            void on_pushOK_clicked();

        private:
            MessageBoxType _type;
            MessageBoxResponse result;
            QString _message;
            QString _id;
            QString _wt;
            Ui::MessageBox *ui;
    };
}

#endif // MESSAGEBOX_H
