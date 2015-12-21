//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef PREFERENCESWIN_H
#define PREFERENCESWIN_H

#include "../libcore/definitions.h"

#include <QHash>
#include <QCheckBox>
#include <QTableWidget>
#include <QComboBox>
#include <QList>
#include <QDialog>

namespace Ui
{
    class PreferencesWin;
}

namespace GrumpyIRC
{
    class Highlighter;
    class Scrollback;
    class Skin;

    class PreferencesWin : public QDialog
    {
            Q_OBJECT

        public:
            explicit PreferencesWin(QWidget *parent = 0);
            ~PreferencesWin();
        private slots:
            void on_buttonBox_rejected();
            void on_buttonBox_accepted();
            void on_tableWidget_cellChanged(int row, int column);
            void on_tableWidget_2_cellChanged(int row, int column);
            void on_tableWidget_customContextMenuRequested(const QPoint &pos);
            void OnHLEnable(bool checked);
            void OnHLRegex(bool checked);
            void on_comboBox_currentIndexChanged(int index);
            void on_pushButton_clicked();
            void on_pushButton_3_clicked();
            void on_pushButton_2_clicked();
            void on_pushButton_4_clicked();
            void on_pushButton_5_clicked();

            void on_pushButton_SC_clicked();

        private:
            void highlights_reload();
            void highlights_append_row(int row, Highlighter *hl);
            QList<int> selectedHLRows();
            void updateSkin();
            void refreshSkin(bool enabled);
            QHash<QTableWidgetItem*, Highlighter*> highlights_source;
            QHash<QCheckBox*, Highlighter*> highlights_enabled;
            QHash<QCheckBox*, Highlighter*> highlights_regex;
            QHash<QComboBox*, Highlighter*> highlights_type;
            Skin *highlighted_skin = nullptr;
            Ui::PreferencesWin *ui;
    };
}

#endif // PREFERENCESWIN_H
