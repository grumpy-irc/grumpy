//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2019

#ifndef SCRIPTFORM_HPP
#define SCRIPTFORM_HPP

#include "../grumpy_global.h"
#include <QHash>
#include <QDialog>

namespace Ui
{
    class ScriptForm;
}

namespace GrumpyIRC
{
    class JSHighlighter;
    class GrumpydSession;

    class ScriptForm : public QDialog
    {
            Q_OBJECT

        public:
            explicit ScriptForm(QWidget *parent = nullptr, GrumpydSession *gsession = nullptr);
            ~ScriptForm();
            void EditScript(QString path, QString script_name);

        private slots:
            void on_pushButton_2_clicked();
            void on_pushButton_clicked();
            void ScriptError(const QHash<QString, QVariant> &info);
            void ScriptGet(const QHash<QString, QVariant> &info);
            void ScriptLoaded(const QHash<QString, QVariant> &info);
            void ScriptRm(const QHash<QString, QVariant> &info);

        private:
            void showError(QString detail);
            void dropRemote();
            void installRemote();
            GrumpydSession *remoteSession;
            QString editingName;
            bool editing = false;
            JSHighlighter *highlighter = nullptr;
            Ui::ScriptForm *ui;
    };
}

#endif // SCRIPTFORM_HPP
