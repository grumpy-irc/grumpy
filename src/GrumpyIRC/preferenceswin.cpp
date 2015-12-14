//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "../libcore/definitions.h"

#include <QMenu>
#include "../libcore/autocompletionengine.h"
#include "../libcore/commandprocessor.h"
#include "../libcore/exception.h"
#include "../libcore/configuration.h"
#include "../libcore/core.h"
#include "../libcore/highlighter.h"
#include "../libcore/generic.h"
#include "preferenceswin.h"
#include "ui_preferenceswin.h"
#include "corewrapper.h"
#include "grumpyconf.h"

using namespace GrumpyIRC;

PreferencesWin::PreferencesWin(QWidget *parent) : QDialog(parent), ui(new Ui::PreferencesWin)
{
    this->ui->setupUi(this);
    this->ui->leIdent->setText(CONF->GetIdent());
    this->ui->leNick->setText(CONF->GetNick());
    this->ui->leNickFix->setText(CONF->GetAlterNick());
    this->ui->lineEdit->setText(CONF->GetRawQuitMessage());
    this->ui->lineEdit_2->setText(CONF->GetName());
    this->ui->lineEdit_3->setText(QString::number(CONF->GetSplitMaxSize()));
    this->ui->checkBox->setChecked(CONF->GetIgnoreSSLProblems());
    this->ui->checkBoxSplitMs->setChecked(CONF->GetSplit());
    this->ui->plainTextEditAutorun->setPlainText(CONF->GetAutorun());

    QStringList heading_1;
    heading_1 << "Highlighted text" << "Is regex" << "Matching" << "Enabled";
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->setColumnCount(heading_1.size());
    this->ui->tableWidget->setShowGrid(false);
    this->ui->tableWidget->setHorizontalHeaderLabels(heading_1);

    QStringList heading_2;
    heading_2 << "Ignored text" << "Is regex" << "Matching" << "Enabled";
    this->ui->tableWidget_2->verticalHeader()->setVisible(false);
    this->ui->tableWidget_2->setColumnCount(heading_2.size());
    this->ui->tableWidget_2->setShowGrid(false);
    this->ui->tableWidget_2->setHorizontalHeaderLabels(heading_2);

    this->highlights_reload();
    this->ui->tableWidget->resizeColumnsToContents();
    this->ui->tableWidget->resizeRowsToContents();
    this->ui->tableWidget_2->resizeColumnsToContents();
}

PreferencesWin::~PreferencesWin()
{
    delete this->ui;
}

void GrumpyIRC::PreferencesWin::on_buttonBox_rejected()
{
    this->close();
}

void GrumpyIRC::PreferencesWin::on_buttonBox_accepted()
{
    CONF->SetAlterNick(this->ui->leNickFix->text());
    CONF->SetNick(this->ui->leNick->text());
    CONF->SetIdent(this->ui->leIdent->text());
    CONF->SetQuitMessage(this->ui->lineEdit->text());
    CONF->SetName(this->ui->lineEdit_2->text());
    CONF->SetIgnoreSSLProblems(this->ui->checkBox->isChecked());
    CONF->SetSplitMaxSize(this->ui->lineEdit_3->text().toInt());
    CONF->SetSplit(this->ui->checkBoxSplitMs->isChecked());
    CONF->SetAutorun(this->ui->plainTextEditAutorun->toPlainText());
    CoreWrapper::GrumpyCore->GetCommandProcessor()->LongSize = CONF->GetSplitMaxSize();
    CoreWrapper::GrumpyCore->GetCommandProcessor()->SplitLong = CONF->GetSplit();
    CONF->Save();
    this->close();
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
    this->ui->tableWidget->insertRow(row);
    QTableWidgetItem *item = new QTableWidgetItem(hl->GetDefinition());
    this->highlights_source.insert(item, hl);
    this->ui->tableWidget->setItem(row, 0, item);

    // Create a check box for regex item
    QCheckBox *regex = new QCheckBox(this);
    regex->setChecked(hl->IsRegex);
    this->highlights_regex.insert(regex, hl);
    this->ui->tableWidget->setCellWidget(row, 1, regex);
    connect(regex, SIGNAL(toggled(bool)), this, SLOT(OnHLRegex(bool)));

    // Combo box for highlight type
    QComboBox *hlt = new QComboBox(this);
    hlt->addItem("Text of message");
    hlt->setCurrentIndex(0);
    this->highlights_type.insert(hlt, hl);
    this->ui->tableWidget->setCellWidget(row, 2, hlt);

    QCheckBox *enabled = new QCheckBox(this);
    enabled->setChecked(hl->Enabled);
    this->highlights_enabled.insert(enabled, hl);
    connect(enabled, SIGNAL(toggled(bool)), this, SLOT(OnHLEnable(bool)));
    this->ui->tableWidget->setCellWidget(row, 3, enabled);
    this->ui->tableWidget->resizeRowToContents(row);
}

QList<int> PreferencesWin::selectedHLRows()
{
    QList<int> results;
    QList<QTableWidgetItem*> items = this->ui->tableWidget->selectedItems();
    foreach (QTableWidgetItem *xx, items)
    {
        if (!results.contains(xx->row()))
            results.append(xx->row());
    }

    return results;
}

void PreferencesWin::on_tableWidget_cellChanged(int row, int column)
{
    QTableWidgetItem *item = this->ui->tableWidget->item(row, column);
    if (!this->highlights_source.contains(item))
        return;

    this->highlights_source[item]->SetDefinition(item->text());
}

void PreferencesWin::on_tableWidget_2_cellChanged(int row, int column)
{

}

void PreferencesWin::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = this->ui->tableWidget->viewport()->mapToGlobal(pos);
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
        this->highlights_append_row(this->ui->tableWidget->rowCount(), hx);
    }
    else if (selectedItem == menuRemove)
    {
        // We need to remove them from last to first
        QList<int> items = this->selectedHLRows();
        qSort(items);
        while (!items.isEmpty())
        {
            int row = items.last();
            items.removeLast();
            QTableWidgetItem *item = this->ui->tableWidget->item(row, 0);
            if (this->highlights_source.contains(item))
            {
                Highlighter *highlight_sel = this->highlights_source[item];
                // Remove the row
                this->ui->tableWidget->removeRow(row);
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

void PreferencesWin::OnHLRegex(bool checked)
{
    QCheckBox *source = (QCheckBox*)QObject::sender();
    if (!this->highlights_regex.contains(source))
        throw new Exception("Checkbox not found", BOOST_CURRENT_FUNCTION);

    this->highlights_regex[source]->IsRegex = checked;
}
