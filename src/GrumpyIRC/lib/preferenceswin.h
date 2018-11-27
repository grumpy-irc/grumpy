//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef PREFERENCESWIN_H
#define PREFERENCESWIN_H

#include "grumpy_global.h"
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

    class LIBGRUMPYSHARED_EXPORT PreferencesWin : public QDialog
    {
            Q_OBJECT

        public:
            explicit PreferencesWin(QWidget *parent = nullptr);
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
            void on_pushButton_WC_clicked();
            void on_pushButton_EC_clicked();
            void on_pushButton_IC_clicked();
            void on_pushButton_AC_clicked();
            void on_pushButton_CQ_clicked();
            void on_pushButton_CA_clicked();
            void on_pushButton_CO_clicked();
            void on_pushButton_CH_clicked();
            void on_pushButton_CV_clicked();
            void on_pushButton_UC_clicked();
            void on_pushButton_HC_clicked();
            void on_pushButton_C1_clicked();
            void on_pushButton_C2_clicked();
            void on_pushButton_C3_clicked();
            void on_pushButton_C4_clicked();
            void on_pushButton_C5_clicked();
            void on_pushButton_C6_clicked();
            void on_pushButton_C7_clicked();
            void on_pushButton_C8_clicked();
            void on_pushButton_C9_clicked();
            void on_pushButton_C10_clicked();
            void on_pushButton_C11_clicked();
            void on_pushButton_C12_clicked();
            void on_pushButton_C13_clicked();
            void on_pushButton_C14_clicked();
            void on_pushButton_C15_clicked();
            void on_pushButton_TC_clicked();
            void on_pushButton_UC_2_clicked();
            void on_pushButton_LC_clicked();
    private:
            void highlights_reload();
            void updateColor();
            void highlights_append_row(int row, Highlighter *hl);
            QList<int> selectedHLRows();
            void updateSkin();
            void refreshSkin(bool enabled);
            QHash<QPushButton*,QColor*> skin_ht;
            QHash<QTableWidgetItem*, Highlighter*> highlights_source;
            QHash<QCheckBox*, Highlighter*> highlights_enabled;
            QHash<QCheckBox*, Highlighter*> highlights_regex;
            QHash<QComboBox*, Highlighter*> highlights_type;
            Skin *highlighted_skin = nullptr;
            Ui::PreferencesWin *ui;
    };
}

#endif // PREFERENCESWIN_H
