//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2019

#include "scriptingmanager.h"
#include "../messagebox.h"
#include "scriptform.h"
#include "ui_scriptingmanager.h"
#include "uiscript.h"
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTimer>
#include <QMenu>
#include <libcore/exception.h>
#include <libcore/grumpydsession.h>
#include <libcore/generic.h>
#include <libcore/scripting/scriptextension.h>

using namespace GrumpyIRC;

ScriptingManager::ScriptingManager(QWidget *parent, GrumpydSession *remote) : QDialog(parent), ui(new Ui::ScriptingManager)
{
    this->remoteSession = remote;
    this->ui->setupUi(this);
    QStringList headers;
    if (remote != nullptr)
    {
        this->setWindowTitle(this->windowTitle() + " (remote)");
    }
    headers << "Name" << "Author" << "Version" << "Is working" << "Description" << "Path";
    this->ui->tableWidget->setColumnCount(headers.count());
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setHorizontalHeaderLabels(headers);
    this->ui->tableWidget->setShowGrid(false);
    this->ui->tableWidget->resizeRowsToContents();
    if (this->remoteSession == nullptr)
    {
        this->Reload();
    } else
    {
        connect(this->remoteSession, SIGNAL(Event_ScriptDeleted(const QHash<QString, QVariant>&)), this, SLOT(OnScriptChange(const QHash<QString, QVariant>&)));
        connect(this->remoteSession, SIGNAL(Event_ScriptInstalled(const QHash<QString, QVariant>&)), this, SLOT(OnScriptChange(const QHash<QString, QVariant>&)));
        this->lastRefresh = this->remoteSession->GetLastUpdateOfScripts();
        this->remoteSession->SendProtocolCommand(GP_CMD_SYS_LIST_SCRIPT);
        this->grumpydRefresh = new QTimer();
        connect(this->grumpydRefresh, SIGNAL(timeout()), this, SLOT(OnTimer()));
        this->grumpydRefresh->start(100);
    }
}

ScriptingManager::~ScriptingManager()
{
    delete this->grumpydRefresh;
    delete this->ui;
}

void ScriptingManager::Reload()
{
    while(this->ui->tableWidget->rowCount())
    {
        this->ui->tableWidget->removeRow(0);
    }

    if (this->remoteSession == nullptr)
    {
        foreach (ScriptExtension *sx, ScriptExtension::GetExtensions())
        {
            int row = this->ui->tableWidget->rowCount();
            this->ui->tableWidget->insertRow(row);
            this->ui->tableWidget->setItem(row, 0, new QTableWidgetItem(sx->GetName()));
            this->ui->tableWidget->setItem(row, 1, new QTableWidgetItem(sx->GetAuthor()));
            this->ui->tableWidget->setItem(row, 2, new QTableWidgetItem(sx->GetVersion()));
            this->ui->tableWidget->setItem(row, 3, new QTableWidgetItem(Generic::Bool2String(sx->IsWorking())));
            this->ui->tableWidget->setItem(row, 4, new QTableWidgetItem(sx->GetDescription()));
            this->ui->tableWidget->setItem(row, 5, new QTableWidgetItem(sx->GetPath()));
        }
    } else
    {
        // Load a list of scripts loaded in remote grumpyd session
        QList<QVariant> scripts = this->remoteSession->ScriptList;
        foreach (QVariant script, scripts)
        {
            QHash<QString, QVariant> info = script.toHash();
            int row = this->ui->tableWidget->rowCount();
            this->ui->tableWidget->insertRow(row);
            this->ui->tableWidget->setItem(row, 0, new QTableWidgetItem(info["name"].toString()));
            this->ui->tableWidget->setItem(row, 1, new QTableWidgetItem(info["author"].toString()));
            this->ui->tableWidget->setItem(row, 2, new QTableWidgetItem(info["version"].toString()));
            this->ui->tableWidget->setItem(row, 3, new QTableWidgetItem(Generic::Bool2String(info["is_working"].toBool())));
            this->ui->tableWidget->setItem(row, 4, new QTableWidgetItem(info["description"].toString()));
            this->ui->tableWidget->setItem(row, 5, new QTableWidgetItem(info["id"].toString()));
        }
    }
    this->ui->tableWidget->resizeColumnsToContents();
    this->ui->tableWidget->resizeRowsToContents();
}

void ScriptingManager::LoadFile(QString path)
{
    if (this->remoteSession != nullptr)
    {
        MessageBox::Error("not-implemented", "Not implemented");
        // todo
        return;
    }
    UiScript *script = new UiScript();
    QString er;
    if (!script->Load(path, &er))
    {
        MessageBox::Error("Failed to load", "Unable to load script " + path + ": " + er, this);
        delete script;
        return;
    }
}

void ScriptingManager::OnScriptChange(const QHash<QString, QVariant> &script_info)
{
    Q_UNUSED(script_info);
    this->remoteSession->SendProtocolCommand(GP_CMD_SYS_LIST_SCRIPT);
}

void ScriptingManager::on_bLoad_clicked()
{
    QFileDialog file_dialog(this);
    file_dialog.setNameFilter("Java script (*.js);;All files (*)");
    file_dialog.setWindowTitle("Open script files");
    file_dialog.setFileMode(QFileDialog::FileMode::ExistingFiles);
    if (file_dialog.exec() == QDialog::DialogCode::Rejected)
        return;
    QStringList files = file_dialog.selectedFiles();
    foreach (QString s, files)
        this->LoadFile(s);
    this->Reload();
}

void ScriptingManager::on_bReload_clicked()
{
    if (this->remoteSession != nullptr)
    {
        MessageBox::Error("not-implemented", "Not implemented");
        // todo
        return;
    }
    QList<ScriptExtension*> old_scripts = ScriptExtension::GetExtensions();
    foreach (ScriptExtension *script, old_scripts)
    {
        QString name = script->GetName();
        QString path = script->GetPath();
        script->Unload();
        delete script;
        QString error;
        UiScript *s = new UiScript();
        if (!s->Load(path, &error))
        {
            MessageBox::Error("Failed to reload script", "Failed to reload " + name + ": " + error, this);
            delete s;
            continue;
        }
    }
    this->Reload();
}

void ScriptingManager::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    QPoint global = this->ui->tableWidget->mapToGlobal(pos);
    QAction *edit = new QAction("Edit", &menu);
    QAction *unload = new QAction("Unload", &menu);
    QAction *reload = new QAction("Reload", &menu);
    QAction *delete_file = new QAction("Delete from disk", &menu);
    menu.addAction(unload);
    menu.addAction(reload);
    menu.addSeparator();
    menu.addAction(delete_file);
    menu.addSeparator();
    menu.addAction(edit);
    if (this->remoteSession)
        unload->setEnabled(false);
    QAction *selection = menu.exec(global);
    if (selection == unload)
    {
        this->unloadSelectSc();
    } else if (selection == delete_file)
    {
        this->deleteSelectSc();
    } else if (selection == reload)
    {
        this->reloadSelectSc();
    } else if (selection == edit)
    {
        QList<int> selected_sc = this->selectedRows();
        if (selected_sc.count() != 1)
        {
            MessageBox::Error("Error", "Please select 1 script to edit", this);
            return;
        }
        QString script_name = this->ui->tableWidget->item(selected_sc[0], 0)->text();
        ScriptExtension *script = ScriptExtension::GetExtensionByName(script_name);
        if (!script)
        {
            MessageBox::Error("Error", "Unable to edit " + script_name + " script not found in memory", this);
            return;
        }
        ScriptForm sf;
        sf.EditScript(script->GetPath(), script->GetName());
        sf.exec();
        this->Reload();
    }
}

void ScriptingManager::on_pushScript_clicked()
{
    ScriptForm sf(this, this->remoteSession);
    sf.exec();
    this->Reload();
}

void ScriptingManager::OnTimer()
{
    if (!this->remoteSession)
        throw new NullPointerException("this->remoteSession", BOOST_CURRENT_FUNCTION);

    if (this->lastRefresh == this->remoteSession->GetLastUpdateOfScripts())
        return;

    this->lastRefresh = this->remoteSession->GetLastUpdateOfScripts();

    // Refresh the list of scripts we have new information from remote
    this->Reload();
}

void ScriptingManager::unloadSelectSc()
{
    if (this->remoteSession != nullptr)
    {
        MessageBox::Error("not-implemented", "Not implemented");
        // todo
        return;
    }
    QList<int> selected = selectedRows();
    foreach (int i, selected)
    {
        QString script_name = this->ui->tableWidget->item(i, 0)->text();
        ScriptExtension *script = ScriptExtension::GetExtensionByName(script_name);
        if (!script)
        {
            MessageBox::Error("Error", "Unable to unload " + script_name + " script not found in memory", this);
            continue;
        }
        script->Unload();
        delete script;
    }
    this->Reload();
}

void ScriptingManager::deleteSelectSc()
{
    QList<int> selected = selectedRows();
    if (selected.isEmpty())
        return;

    if (MessageBox::Question("scr", "Delete files", "Are you sure you want to permanently delete selected files?", this) == MessageBoxResponse_No)
        return;

    if (this->remoteSession != nullptr)
    {
        foreach (int i, selected)
        {
            QHash<QString, QVariant> info;
            info.insert("name", this->ui->tableWidget->item(i, 0)->text());
            this->remoteSession->SendProtocolCommand(GP_CMD_SYS_UNINST_SCRIPT, info);
        }
        return;
    }

    foreach (int i, selected)
    {
        QString script_name = this->ui->tableWidget->item(i, 0)->text();
        ScriptExtension *script = ScriptExtension::GetExtensionByName(script_name);
        if (!script)
        {
            MessageBox::Error("Error", "Unable to unload " + script_name + " script not found in memory", this);
            continue;
        }
        QString path = script->GetPath();
        script->Unload();
        delete script;
        QFile file(path);
        if (!file.remove())
            MessageBox::Error("Error", "Unable to remove " + path, this);
    }
    this->Reload();
}

void ScriptingManager::reloadSelectSc()
{
    if (this->remoteSession != nullptr)
    {
        MessageBox::Error("not-implemented", "Not implemented");
        // todo
        return;
    }

    QList<int> selected = selectedRows();
    foreach (int i, selected)
    {
        QString script_name = this->ui->tableWidget->item(i, 0)->text();
        QString error;
        ScriptExtension *script = ScriptExtension::GetExtensionByName(script_name);
        if (!script)
        {
            MessageBox::Error("Error", "Unable to reload " + script_name + " script not found in memory", this);
            continue;
        }
        QString file = script->GetPath();
        script->Unload();
        delete script;
        UiScript *s = new UiScript();
        if (!s->Load(file, &error))
        {
            MessageBox::Error("Failed to reload script", "Failed to reload " + script_name + ": " + error, this);
            delete s;
            continue;
        }
    }
    this->Reload();
}

QList<int> ScriptingManager::selectedRows()
{
    QList<int> selection;
    foreach (QTableWidgetItem *i, this->ui->tableWidget->selectedItems())
    {
        if (!selection.contains(i->row()))
            selection.append(i->row());
    }
    return selection;
}
