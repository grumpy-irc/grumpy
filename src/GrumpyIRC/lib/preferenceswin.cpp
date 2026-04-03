//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include <libcore/definitions.h>
#include <algorithm>  // Add at top with other includes

#include "preferenceswin.h"
#include "skin.h"
#include "ui_preferenceswin.h"
#include "scrollbackframe.h"
#include "mainwindow.h"
#include "corewrapper.h"
#include "grumpyconf.h"
#include <QMenu>
#include <QColorDialog>
#include <QMessageBox>
#include <libcore/autocompletionengine.h>
#include <libcore/commandprocessor.h>
#include <libcore/exception.h>
#include <libcore/configuration.h>
#include <libcore/core.h>
#include <libcore/highlighter.h>
#include <libcore/generic.h>

using namespace GrumpyIRC;

PreferencesWin::PreferencesWin(QWidget *parent) : QDialog(parent), ui(new Ui::PreferencesWin)
{
    this->ui->setupUi(this);
    this->ui->leIdent->setText(CONF->GetIdent());
    this->ui->leNick->setText(CONF->GetNick());
    this->ui->leNickFix->setText(CONF->GetAlterNick());
    this->ui->lineEditQuitMessage->setText(CONF->GetRawQuitMessage());
    this->ui->lineEditRealName->setText(CONF->GetName());
    this->ui->lineEditSplitMaxSize->setText(QString::number(CONF->GetSplitMaxSize()));
    this->ui->checkBoxIgnoreInvalidSsl->setChecked(CONF->GetIgnoreSSLProblems());
    this->ui->checkBoxAutoReduceSplitSize->setChecked(CONF->GetAutoReduceMaxSendSize());
    this->ui->checkBoxSplitLongMessages->setChecked(CONF->GetSplit());
    this->ui->checkBoxShowColorPreview->setChecked(CONF->GetColorBoxShow());
    this->ui->plainTextEditAutorun->setPlainText(CONF->GetAutorun());
    this->ui->lineEdit_ChannelH->setText(CONF->GetChannelHeader());
    this->ui->lineEdit_LabeledH->setText(CONF->GetLabeledHeader());
    this->ui->lineEdit_StandardH->setText(CONF->GetStandardHeader());
    this->ui->le_Mask->setText(CONF->GetDefaultBanMask());
    this->ui->lineEdit_Kick->setText(CONF->GetDefaultKickReason());
    this->ui->lineEdit_AutoAway->setText(QString::number(CONF->GetAutoAwayTime()));
    this->ui->lineEdit_Away->setText(CONF->GetAutoAwayMsg());
    this->ui->checkBoxAutoAwayEnabled->setChecked(CONF->GetAutoAway());
    this->ui->checkBoxAllowUnsafeScripts->setChecked(CONF->GetConfiguration()->GetUnsafeScriptFc());
    QString ignored;
    foreach (int numeric, CONF->IgnoredNums())
    {
        ignored += QString::number(numeric) + ", ";
    }
    ignored = ignored.trimmed();
    this->ui->lineEditIgnoredNumerics->setText(ignored);

    QStringList heading_1;
    heading_1 << "Highlighted text" << "Is regex" << "Matching" << "Enabled";
    this->ui->tableWidgetHighlights->verticalHeader()->setVisible(false);
    this->ui->tableWidgetHighlights->setColumnCount(heading_1.size());
    this->ui->tableWidgetHighlights->setShowGrid(false);
    this->ui->tableWidgetHighlights->setHorizontalHeaderLabels(heading_1);

    QStringList heading_2;
    heading_2 << "Ignored text" << "Is regex" << "Matching" << "Enabled";
    this->ui->tableWidgetIgnoredText->verticalHeader()->setVisible(false);
    this->ui->tableWidgetIgnoredText->setColumnCount(heading_2.size());
    this->ui->tableWidgetIgnoredText->setShowGrid(false);
    this->ui->tableWidgetIgnoredText->setHorizontalHeaderLabels(heading_2);

    this->highlights_reload();
    this->ui->tableWidgetHighlights->resizeColumnsToContents();
    this->ui->tableWidgetHighlights->resizeRowsToContents();
    this->ui->tableWidgetIgnoredText->resizeColumnsToContents();
    this->updateSkin();

    this->ui->lineEdit_FormatActn->setText(CONF->GetActionFormat());
    this->ui->lineEdit_FormatMsg->setText(CONF->GetMessageFormat());
    this->ui->lineEdit_FormatNt->setText(CONF->GetNoticeFormat());
    this->ui->lineEdit_FormatText->setText(CONF->GetLineFormat());

    this->ui->comboBoxEncoding->addItem("Default");
    this->ui->comboBoxEncoding->addItem("ASCII");
    this->ui->comboBoxEncoding->addItem("UTF-8");
    this->ui->comboBoxEncoding->addItem("UTF-16");
    this->ui->comboBoxEncoding->addItem("Latin");

    switch (CONF->GetEncoding())
    {
        case libircclient::EncodingDefault:
            this->ui->comboBoxEncoding->setCurrentIndex(0);
            break;
        case libircclient::EncodingASCII:
            this->ui->comboBoxEncoding->setCurrentIndex(1);
            break;
        case libircclient::EncodingUTF8:
            this->ui->comboBoxEncoding->setCurrentIndex(2);
            break;
        case libircclient::EncodingUTF16:
            this->ui->comboBoxEncoding->setCurrentIndex(3);
            break;
        case libircclient::EncodingLatin:
            this->ui->comboBoxEncoding->setCurrentIndex(4);
            break;
    }

    this->ui->lineEditContinuousLoggingPath->setText(CONF->GetContinuousLoggingPath());
    this->ui->checkBoxContinuousLogging->setChecked(CONF->GetContinuousLoggingEnabled());
}

PreferencesWin::~PreferencesWin()
{
    delete this->ui;
}

void PreferencesWin::accept()
{
    if (!this->confirmPendingSkinChanges())
        return;

    CONF->SetAlterNick(this->ui->leNickFix->text());
    CONF->SetNick(this->ui->leNick->text());
    CONF->SetIdent(this->ui->leIdent->text());
    CONF->SetQuitMessage(this->ui->lineEditQuitMessage->text());
    CONF->SetName(this->ui->lineEditRealName->text());
    CONF->SetIgnoreSSLProblems(this->ui->checkBoxIgnoreInvalidSsl->isChecked());
    CONF->SetSplitMaxSize(this->ui->lineEditSplitMaxSize->text().toInt());
    CONF->SetSplit(this->ui->checkBoxSplitLongMessages->isChecked());
    CONF->SetAutorun(this->ui->plainTextEditAutorun->toPlainText());
    CONF->SetAutoReduceMaxSendSize(this->ui->checkBoxAutoReduceSplitSize->isChecked());
    CoreWrapper::GrumpyCore->GetCommandProcessor()->LongSize = CONF->GetSplitMaxSize();
    CoreWrapper::GrumpyCore->GetCommandProcessor()->SplitLong = CONF->GetSplit();
    CoreWrapper::GrumpyCore->GetCommandProcessor()->AutoReduceMsgSize = CONF->GetAutoReduceMaxSendSize();
    CONF->SetChannelH(this->ui->lineEdit_ChannelH->text());
    CONF->SetLabeledH(this->ui->lineEdit_LabeledH->text());
    CONF->SetStandardH(this->ui->lineEdit_StandardH->text());
    CONF->SetMessageFormat(this->ui->lineEdit_FormatMsg->text());
    CONF->SetLineFormat(this->ui->lineEdit_FormatText->text());
    CONF->SetColorBoxShow(this->ui->checkBoxShowColorPreview->isChecked());
    CONF->SetDefaultBanMask(this->ui->le_Mask->text());
    CONF->SetDefaultKickReason(this->ui->lineEdit_Kick->text());
    CONF->SetAutoAwayMsg(this->ui->lineEdit_Away->text());
    CONF->SetAutoAway(this->ui->checkBoxAutoAwayEnabled->isChecked());
    CONF->SetAutoAwayTime(this->ui->lineEdit_AutoAway->text().toInt());
    CONF->SetContinuousLoggingPath(this->ui->lineEditContinuousLoggingPath->text());
    CONF->SetContinuousLoggingEnabled(this->ui->checkBoxContinuousLogging->isChecked());
    QList<int> ignored_nums;
    QList<QString> ignored = this->ui->lineEditIgnoredNumerics->text().split(",");
    foreach (QString numeric, ignored)
    {
        numeric = numeric.trimmed();
        if (numeric.isEmpty())
            continue;
        ignored_nums.append(numeric.toInt());
    }
    Skin::Current = this->highlighted_skin;
    CONF->SetIRCIgnoredNumerics(ignored_nums);
    ScrollbackFrame::UpdateSkins();
    switch(this->ui->comboBoxEncoding->currentIndex())
    {
        case 0:
            CONF->SetEncoding(libircclient::EncodingDefault);
            break;
        case 1:
            CONF->SetEncoding(libircclient::EncodingASCII);
            break;
        case 2:
            CONF->SetEncoding(libircclient::EncodingUTF8);
            break;
        case 3:
            CONF->SetEncoding(libircclient::EncodingUTF16);
            break;
        case 4:
            CONF->SetEncoding(libircclient::EncodingLatin);
            break;
    }
    CONF->GetConfiguration()->SetUnsafeScriptFc(this->ui->checkBoxAllowUnsafeScripts->isChecked());
    CONF->Save();
    MainWindow::Main->SetupAutoAway();
    MainWindow::Main->UpdateSkin();

    QDialog::accept();
}

bool PreferencesWin::hasPendingSkinChanges() const
{
    if (!this->highlighted_skin || this->highlighted_skin->IsDefault())
        return false;

    return this->ui->lineEditSkinName->text() != this->highlighted_skin->Name
            || this->ui->lineEditSkinTextSize->text().toInt() != this->highlighted_skin->TextSize
            || this->ui->lineEditSkinFontFamily->text() != this->highlighted_skin->FontFamily
            || this->ui->lineEditSkinBackgroundImage->text() != this->highlighted_skin->BackgroundImage
            || this->ui->sliderSkinOpacity->value() != this->highlighted_skin->Opacity;
}

bool PreferencesWin::confirmPendingSkinChanges()
{
    if (!this->hasPendingSkinChanges())
        return true;

    this->ui->tabWidgetPreferences->setCurrentWidget(this->ui->tabSkinColors);
    this->ui->pushButtonSaveSkin->setFocus();

    QMessageBox messageBox;
    messageBox.setWindowTitle("Unsaved skin changes");
    messageBox.setText("The selected skin has unsaved changes.");
    messageBox.setInformativeText("Do you want to abort saving Preferences and return to the skin editor?");
    messageBox.setIcon(QMessageBox::Warning);
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    messageBox.setDefaultButton(QMessageBox::Yes);
    messageBox.button(QMessageBox::Yes)->setText("Abort saving");
    messageBox.button(QMessageBox::No)->setText("Continue");

    int result = messageBox.exec();
    return result == QMessageBox::No;
}

void PreferencesWin::saveSkinChanges()
{
    if (!this->highlighted_skin || this->highlighted_skin->IsDefault())
        return;

    this->highlighted_skin->Name = this->ui->lineEditSkinName->text();
    this->highlighted_skin->TextSize = this->ui->lineEditSkinTextSize->text().toInt();
    this->highlighted_skin->FontFamily = this->ui->lineEditSkinFontFamily->text();
    this->highlighted_skin->BackgroundImage = this->ui->lineEditSkinBackgroundImage->text();
    this->highlighted_skin->Opacity = this->ui->sliderSkinOpacity->value();
}

void PreferencesWin::highlights_reload()
{
    // This will leak until the window is closed, but that's most likely not a big problem
    this->highlights_enabled.clear();
    this->highlights_regex.clear();
    this->highlights_type.clear();
    int row = 0;
    foreach (Highlighter *hl, Highlighter::Highlighter_Data)
    {
        this->highlights_append_row(row++, hl);
    }
}

void PreferencesWin::highlights_append_row(int row, Highlighter *hl)
{
    this->ui->tableWidgetHighlights->insertRow(row);
    QTableWidgetItem *item = new QTableWidgetItem(hl->GetDefinition());
    this->highlights_source.insert(item, hl);
    this->ui->tableWidgetHighlights->setItem(row, 0, item);

    // Create a check box for regex item
    QCheckBox *regex = new QCheckBox(this);
    regex->setChecked(hl->IsRegex);
    this->highlights_regex.insert(regex, hl);
    this->ui->tableWidgetHighlights->setCellWidget(row, 1, regex);
    connect(regex, SIGNAL(toggled(bool)), this, SLOT(OnHLRegex(bool)));

    // Combo box for highlight type
    QComboBox *hlt = new QComboBox(this);
    hlt->addItem("Text of message");
    hlt->setCurrentIndex(0);
    this->highlights_type.insert(hlt, hl);
    this->ui->tableWidgetHighlights->setCellWidget(row, 2, hlt);

    QCheckBox *enabled = new QCheckBox(this);
    enabled->setChecked(hl->Enabled);
    this->highlights_enabled.insert(enabled, hl);
    connect(enabled, SIGNAL(toggled(bool)), this, SLOT(OnHLEnable(bool)));
    this->ui->tableWidgetHighlights->setCellWidget(row, 3, enabled);
    this->ui->tableWidgetHighlights->resizeRowToContents(row);
}

QList<int> PreferencesWin::selectedHLRows()
{
    QList<int> results;
    QList<QTableWidgetItem*> items = this->ui->tableWidgetHighlights->selectedItems();
    foreach (QTableWidgetItem *xx, items)
    {
        if (!results.contains(xx->row()))
            results.append(xx->row());
    }

    return results;
}

void PreferencesWin::updateSkin()
{
    int selectedSkin = Skin::SkinList.indexOf(Skin::GetCurrent());
    if (this->ui->comboBoxSkin->count() > 0)
        selectedSkin = this->ui->comboBoxSkin->currentIndex();
    this->ui->comboBoxSkin->clear();
    foreach (Skin *skin, Skin::SkinList)
    {
        this->ui->comboBoxSkin->addItem(skin->Name);
    }
    while (selectedSkin >= this->ui->comboBoxSkin->count())
        selectedSkin -= 1;
    this->ui->comboBoxSkin->setCurrentIndex(selectedSkin);
}

void PreferencesWin::refreshSkin(bool enabled)
{
    this->ui->pushButtonSkinBackgroundColor->setEnabled(enabled);
    this->ui->lineEditSkinFontFamily->setEnabled(enabled);
    this->ui->lineEditSkinName->setEnabled(enabled);
    this->ui->pushButtonSkinTextColor->setEnabled(enabled);
    this->ui->lineEditSkinTextSize->setEnabled(enabled);
    this->ui->pushButtonSkinAwayColor->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor1->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor2->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor3->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor4->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor5->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor6->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor7->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor8->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor9->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor10->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor11->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor12->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor13->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor14->setEnabled(enabled);
    this->ui->pushButtonSkinPaletteColor15->setEnabled(enabled);

    this->ui->pushButtonSkinModeAColor->setEnabled(enabled);
    this->ui->pushButtonSkinModeHColor->setEnabled(enabled);
    this->ui->pushButtonSkinModeOColor->setEnabled(enabled);
    this->ui->pushButtonSkinModeQColor->setEnabled(enabled);
    this->ui->pushButtonSkinModeVColor->setEnabled(enabled);
    this->ui->pushButtonSkinErrorColor->setEnabled(enabled);
    this->ui->pushButtonSkinHighlightColor->setEnabled(enabled);
    this->ui->pushButtonSkinSystemInfoColor->setEnabled(enabled);
    this->ui->pushButtonSkinWarningColor->setEnabled(enabled);
    this->ui->pushButtonSkinSystemColor->setEnabled(enabled);
    this->ui->pushButtonSkinUnreadColor->setEnabled(enabled);
    this->ui->pushButtonSkinTimestampColor->setEnabled(enabled);
    this->ui->pushButtonSkinUserColor->setEnabled(enabled);
    this->ui->pushButtonSkinLinkColor->setEnabled(enabled);
    this->ui->sliderSkinOpacity->setEnabled(enabled);
    this->ui->lineEditSkinBackgroundImage->setEnabled(enabled);
}

void PreferencesWin::on_tableWidgetHighlights_cellChanged(int row, int column)
{
    QTableWidgetItem *item = this->ui->tableWidgetHighlights->item(row, column);
    if (!this->highlights_source.contains(item))
        return;

    this->highlights_source[item]->SetDefinition(item->text());
}

void PreferencesWin::on_tableWidgetIgnoredText_cellChanged(int row, int column)
{

}

void PreferencesWin::on_tableWidgetHighlights_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = this->ui->tableWidgetHighlights->viewport()->mapToGlobal(pos);
    QMenu Menu;
    // Items
    QAction *menuInsert = new QAction("Insert", &Menu);
    Menu.addAction(menuInsert);
    QAction *menuRemove = new QAction("Remove", &Menu);
    Menu.addAction(menuRemove);

    QAction* selectedItem = Menu.exec(globalPos);
    if (!selectedItem)
        return;
    if (selectedItem == menuInsert)
    {
        Highlighter *hx = new Highlighter("");
        hx->Enabled = false;
        this->highlights_append_row(this->ui->tableWidgetHighlights->rowCount(), hx);
    }
    else if (selectedItem == menuRemove)
    {
        // We need to remove them from last to first
        QList<int> items = this->selectedHLRows();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        std::sort(items.begin(), items.end());
#else
        qSort(items);
#endif
        while (!items.isEmpty())
        {
            int row = items.last();
            items.removeLast();
            QTableWidgetItem *item = this->ui->tableWidgetHighlights->item(row, 0);
            if (this->highlights_source.contains(item))
            {
                Highlighter *highlight_sel = this->highlights_source[item];
                // Remove the row
                this->ui->tableWidgetHighlights->removeRow(row);
                // Nuke the highlight
                delete highlight_sel;
            }
        }
    }
}

void PreferencesWin::OnHLEnable(bool checked)
{
    QCheckBox *source = (QCheckBox*)QObject::sender();
    if (!this->highlights_enabled.contains(source))
        throw new Exception("Checkbox not found", BOOST_CURRENT_FUNCTION);

    this->highlights_enabled[source]->Enabled = checked;
}

static void UpdateButton(QPushButton *push, QColor color)
{
    QPalette temp = push->palette();
    QColor inverted;
    inverted.setBlue(255 - color.blue());
    inverted.setRed(255 - color.red());
    inverted.setGreen(255 - color.green());
    temp.setColor(QPalette::ButtonText, inverted);
    push->setStyleSheet("background-color: " + color.name() + ";");
    push->setPalette(temp);
}

void PreferencesWin::OnHLRegex(bool checked)
{
    QCheckBox *source = (QCheckBox*)QObject::sender();
    if (!this->highlights_regex.contains(source))
        throw new Exception("Checkbox not found", BOOST_CURRENT_FUNCTION);

    this->highlights_regex[source]->IsRegex = checked;
}

void GrumpyIRC::PreferencesWin::on_comboBoxSkin_currentIndexChanged(int index)
{
    if (index < 0)
        return;

    if (index >= Skin::SkinList.count())
        throw new Exception("Invalid skin_ptr", BOOST_CURRENT_FUNCTION);

    this->highlighted_skin = Skin::SkinList[index];
    this->ui->lineEditSkinFontFamily->setText(this->highlighted_skin->FontFamily);
    this->ui->lineEditSkinTextSize->setText(QString::number(this->highlighted_skin->TextSize));
    this->ui->lineEditSkinName->setText(this->highlighted_skin->Name);
    this->ui->lineEditSkinBackgroundImage->setText(this->highlighted_skin->BackgroundImage);
    this->ui->sliderSkinOpacity->setValue(this->highlighted_skin->Opacity);

    // This is to save us some coding, it's a little bit slower but scales much better:
    // We put a pointer to every button and color of current skin into a hash table
    // then we let the functions pair them
    this->skin_ht.clear();
    this->skin_ht.insert(this->ui->pushButtonSkinBackgroundColor, &this->highlighted_skin->BackgroundColor);
    this->skin_ht.insert(this->ui->pushButtonSkinTextColor, &this->highlighted_skin->TextColor);
    this->skin_ht.insert(this->ui->pushButtonSkinSystemColor, &this->highlighted_skin->SystemColor);
    this->skin_ht.insert(this->ui->pushButtonSkinAwayColor, &this->highlighted_skin->UserListAwayColor);
    this->skin_ht.insert(this->ui->pushButtonSkinErrorColor, &this->highlighted_skin->Error);
    this->skin_ht.insert(this->ui->pushButtonSkinHighlightColor, &this->highlighted_skin->HighligtedColor);
    this->skin_ht.insert(this->ui->pushButtonSkinSystemInfoColor, &this->highlighted_skin->SystemInfo);
    this->skin_ht.insert(this->ui->pushButtonSkinUnreadColor, &this->highlighted_skin->Unread);
    this->skin_ht.insert(this->ui->pushButtonSkinWarningColor, &this->highlighted_skin->Warning);
    this->skin_ht.insert(this->ui->pushButtonSkinTimestampColor, &this->highlighted_skin->Timestamp);
    this->skin_ht.insert(this->ui->pushButtonSkinUserColor, &this->highlighted_skin->UserColor);
    this->skin_ht.insert(this->ui->pushButtonSkinLinkColor, &this->highlighted_skin->LinkColor);
    //this->skin_ht.insert(this->ui->pushButtonSkinModeAColor, &this->highlighted_skin->)

    foreach (QPushButton *button, this->skin_ht.keys())
        UpdateButton(button, *this->skin_ht[button]);

    this->refreshSkin(!this->highlighted_skin->IsDefault());
    //this->ui->lineEdit_SkinBk->setText(skin->BackgroundColor);
}

void GrumpyIRC::PreferencesWin::on_pushButtonSaveSkin_clicked()
{
    this->saveSkinChanges();
}

void GrumpyIRC::PreferencesWin::on_pushButtonRemoveSkin_clicked()
{
    if (this->highlighted_skin->IsDefault())
        return;

    delete this->highlighted_skin;
    this->highlighted_skin = nullptr;
    this->updateSkin();
}

void GrumpyIRC::PreferencesWin::on_pushButtonDuplicateSkin_clicked()
{
    new Skin(this->highlighted_skin);
    this->updateSkin();

    // Now that we created a new skin we can switch it
    this->ui->comboBoxSkin->setCurrentIndex(this->ui->comboBoxSkin->count() - 1);
}

static QColor GetColorUsingPicker(QColor color)
{
    QColor original = color;
    QColorDialog *picker = new QColorDialog(color, nullptr);
    picker->exec();
    color = picker->selectedColor();
    delete picker;
    if (!color.isValid())
        return original;
    return color;
}

void PreferencesWin::updateColor()
{
    if (!this->highlighted_skin)
        return;

    if (!this->skin_ht.contains((QPushButton*)QObject::sender()))
        throw new Exception("Button not found", BOOST_CURRENT_FUNCTION);

    *this->skin_ht[(QPushButton*)QObject::sender()] = GetColorUsingPicker(*this->skin_ht[(QPushButton*)QObject::sender()]);
    UpdateButton((QPushButton*)QObject::sender(), *this->skin_ht[(QPushButton*)QObject::sender()]);
}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinBackgroundColor_clicked()
{
    this->updateColor();
}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinTextColor_clicked()
{
    this->updateColor();
}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinSystemColor_clicked()
{
    this->updateColor();
}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinWarningColor_clicked()
{
    this->updateColor();
}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinErrorColor_clicked()
{
    this->updateColor();
}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinSystemInfoColor_clicked()
{
    this->updateColor();
}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinAwayColor_clicked()
{
    this->updateColor();
}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinModeQColor_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinModeAColor_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinModeOColor_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinModeHColor_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinModeVColor_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinUnreadColor_clicked()
{
    this->updateColor();
}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinHighlightColor_clicked()
{
    this->updateColor();
}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor1_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor2_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor3_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor4_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor5_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor6_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor7_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor8_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor9_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor10_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor11_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor12_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor13_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor14_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinPaletteColor15_clicked()
{

}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinTimestampColor_clicked()
{
    this->updateColor();
}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinUserColor_clicked()
{
    this->updateColor();
}

void GrumpyIRC::PreferencesWin::on_pushButtonSkinLinkColor_clicked()
{
    this->updateColor();
}
