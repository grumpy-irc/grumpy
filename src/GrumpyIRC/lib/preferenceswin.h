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

        public slots:
            void accept() override;

        private slots:
            void on_tableWidgetHighlights_cellChanged(int row, int column);
            void on_tableWidgetIgnoredText_cellChanged(int row, int column);
            void on_tableWidgetHighlights_customContextMenuRequested(const QPoint &pos);
            void OnHLEnable(bool checked);
            void OnHLRegex(bool checked);
            void on_comboBoxSkin_currentIndexChanged(int index);
            void on_pushButtonSaveSkin_clicked();
            void on_pushButtonRemoveSkin_clicked();
            void on_pushButtonDuplicateSkin_clicked();
            void on_pushButtonSkinBackgroundColor_clicked();
            void on_pushButtonSkinTextColor_clicked();
            void on_pushButtonSkinSystemColor_clicked();
            void on_pushButtonSkinWarningColor_clicked();
            void on_pushButtonSkinErrorColor_clicked();
            void on_pushButtonSkinSystemInfoColor_clicked();
            void on_pushButtonSkinAwayColor_clicked();
            void on_pushButtonSkinModeQColor_clicked();
            void on_pushButtonSkinModeAColor_clicked();
            void on_pushButtonSkinModeOColor_clicked();
            void on_pushButtonSkinModeHColor_clicked();
            void on_pushButtonSkinModeVColor_clicked();
            void on_pushButtonSkinUnreadColor_clicked();
            void on_pushButtonSkinHighlightColor_clicked();
            void on_pushButtonSkinPaletteColor1_clicked();
            void on_pushButtonSkinPaletteColor2_clicked();
            void on_pushButtonSkinPaletteColor3_clicked();
            void on_pushButtonSkinPaletteColor4_clicked();
            void on_pushButtonSkinPaletteColor5_clicked();
            void on_pushButtonSkinPaletteColor6_clicked();
            void on_pushButtonSkinPaletteColor7_clicked();
            void on_pushButtonSkinPaletteColor8_clicked();
            void on_pushButtonSkinPaletteColor9_clicked();
            void on_pushButtonSkinPaletteColor10_clicked();
            void on_pushButtonSkinPaletteColor11_clicked();
            void on_pushButtonSkinPaletteColor12_clicked();
            void on_pushButtonSkinPaletteColor13_clicked();
            void on_pushButtonSkinPaletteColor14_clicked();
            void on_pushButtonSkinPaletteColor15_clicked();
            void on_pushButtonSkinTimestampColor_clicked();
            void on_pushButtonSkinUserColor_clicked();
            void on_pushButtonSkinLinkColor_clicked();
    private:
            void highlights_reload();
            bool hasPendingSkinChanges() const;
            bool confirmPendingSkinChanges();
            void saveSkinChanges();
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
