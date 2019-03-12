//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2019

#include "../messagebox.h"
#include "../grumpyconf.h"
#include "scriptform.h"
#include "uiscript.h"
#include "ui_scriptform.h"
#include "jshighlighter.h"
#include <libcore/configuration.h>
#include <libcore/core.h>
#include <libcore/generic.h>
#include <libcore/grumpydsession.h>
#include <libcore/resources.h>
#include <libcore/scripting/scriptextension.h>
#include <QFontDatabase>
#include <QHash>
#include <QFile>
#include <QFileDialog>

using namespace GrumpyIRC;

ScriptForm::ScriptForm(QWidget *parent, GrumpydSession *gsession) : QDialog(parent), ui(new Ui::ScriptForm)
{
    this->remoteSession = gsession;
    this->ui->setupUi(this);
    this->ui->textEdit->setText(Resources::GetSource("/grumpy_core/scripting/ecma/example.js"));
    this->ui->textEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    QString file_name = Core::GrumpyCore->GetConfiguration()->GetHomeScriptPath() + "new_script.js";
    if (this->remoteSession == nullptr)
    {
        int file_id = 1;
        while (QFile(file_name).exists())
        {
            file_name = Core::GrumpyCore->GetConfiguration()->GetHomeScriptPath() + "new_script" + QString::number(file_id++) + ".js";
        }
    } else
    {
        connect(this->remoteSession, SIGNAL(Event_Error(const QHash<QString, QVariant> &)), this, SLOT(ScriptError(const QHash<QString, QVariant> &)));
        file_name = "";
    }
    this->ui->lineEdit_2->setText(file_name);
    this->highlighter = new JSHighlighter(this->ui->textEdit->document());
    if (this->remoteSession != nullptr)
        this->ui->pushButton_2->setEnabled(false);
}

ScriptForm::~ScriptForm()
{
    delete this->highlighter;
    delete this->ui;
}

void ScriptForm::EditScript(QString path, QString script_name)
{
    this->editing = true;
    this->ui->pushButton->setText("Save, reload and close");
    this->editingName = script_name;
    this->ui->lineEdit_2->setEnabled(false);
    // Grumpyd
    if (this->remoteSession)
    {
        this->setWindowTitle("Edit script");
        this->ui->textEdit->setText("Loading source...");
        this->ui->textEdit->setEnabled(false);
        this->ui->lineEdit_2->setText(path);
        connect(this->remoteSession, SIGNAL(Event_ScriptSource(const QHash<QString, QVariant>&)), this, SLOT(ScriptGet(const QHash<QString, QVariant>&)));
        QHash<QString, QVariant> parameters;
        parameters.insert("name", script_name);
        this->remoteSession->SendProtocolCommand(GP_CMD_SYS_READ_SCRIPT_SOURCE_CODE, parameters);
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadWrite))
    {
        MessageBox::Error("Error", "Unable to open " + path + " for writing", this);
        return;
    }
    this->ui->textEdit->setText(QString(file.readAll()));
    this->ui->lineEdit_2->setText(path);
}

void ScriptForm::on_pushButton_2_clicked()
{
    QFileDialog file_dialog(this);
    file_dialog.setNameFilter("Java script (*.js);;All files (*)");
    file_dialog.setWindowTitle("Select script file");
    file_dialog.setFileMode(QFileDialog::FileMode::AnyFile);
    if (file_dialog.exec() == QDialog::DialogCode::Rejected)
        return;
    QString path = file_dialog.selectedFiles().at(0);
    if (!this->remoteSession && !path.isEmpty())
    {
        this->ui->lineEdit_2->setText(path);
    }
}

void ScriptForm::on_pushButton_clicked()
{
    if (this->remoteSession != nullptr)
    {
        this->ui->textEdit->setEnabled(false);
        this->ui->pushButton->setEnabled(false);
        if (this->editing)
        {
            this->dropRemote();
            return;
        }
        this->installRemote();
        return;
    }

    QString fullpath = this->ui->lineEdit_2->text();
    QFile f(fullpath);
    if (!f.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        MessageBox::Error("Error", "Unable to open " + fullpath + " for writing", this);
        return;
    }
    if (f.write(this->ui->textEdit->toPlainText().toUtf8()) < 0)
    {
        MessageBox::Error("Error", "Unable to write - disk full?!", this);
        return;
    }

    f.close();

    // In case we are editing some script, let's unload it now
    if (this->editing)
    {
        ScriptExtension *previous = ScriptExtension::GetExtensionByName(this->editingName);
        if (previous)
        {
            previous->Unload();
            delete previous;
        }
    }

    QString er;
    UiScript *script = new UiScript();
    if (script->Load(fullpath, &er))
    {
        this->close();
    } else
    {
        MessageBox::Error("Error", "Failed to load a JS script: " + er, this);
        delete script;
    }
}

void ScriptForm::ScriptError(const QHash<QString, QVariant> &info)
{
    QString msg = info["description"].toString();
    this->showError("Error from remote: " + msg);
}

void ScriptForm::ScriptGet(const QHash<QString, QVariant> &info)
{
    if (!info.contains("name") || !info.contains("source"))
    {
        MessageBox::Error("Error", "Remote grumpyd didn't return source code for script");
        this->close();
        return;
    }

    if (info["name"].toString() != this->editingName)
        return;

    this->ui->textEdit->setEnabled(true);
    this->ui->textEdit->setText(info["source"].toString());
}

void ScriptForm::ScriptLoaded(const QHash<QString, QVariant> &info)
{
    //if (!info.contains("name"))
    //    return;
    //if (info["name"].toString() != this->editingName)
    //    return;
    this->close();
}

void ScriptForm::ScriptRm(const QHash<QString, QVariant> &info)
{
    if (!info.contains("name"))
        return;
    if (info["name"].toString() != this->editingName)
        return;
    this->installRemote();
}

void ScriptForm::showError(QString detail)
{
    this->ui->pushButton->setText("Save");
    this->ui->pushButton->setEnabled(true);
    this->ui->textEdit->setEnabled(true);
    MessageBox::Error("Error", detail, this);
}

void ScriptForm::dropRemote()
{
    this->ui->pushButton->setText("Removing old script...");
    connect(this->remoteSession, SIGNAL(Event_ScriptDeleted(const QHash<QString, QVariant>&)), this, SLOT(ScriptRm(const QHash<QString, QVariant>&)));
    QHash<QString, QVariant> parameters;
    parameters.insert("name", this->editingName);
    this->remoteSession->SendProtocolCommand(GP_CMD_SYS_UNINST_SCRIPT, parameters);
}

void ScriptForm::installRemote()
{
    this->ui->pushButton->setText("Installing new script...");
    QString path = this->ui->lineEdit_2->text();
    if (path.isEmpty())
    {
        this->showError("No script name provided");
        return;
    }

    if (!path.toLower().endsWith(".js"))
        path += ".js";

    if (!Generic::IsValidFileName(path))
    {
        this->showError("Script name is not valid");
        return;
    }
    QString source = this->ui->textEdit->toPlainText();
    connect(this->remoteSession, SIGNAL(Event_ScriptInstalled(const QHash<QString, QVariant>&)), this, SLOT(ScriptLoaded(const QHash<QString, QVariant>&)));
    QHash<QString, QVariant> parameters;
    parameters.insert("path", QVariant(path));
    parameters.insert("source", QVariant(source));
    this->remoteSession->SendProtocolCommand(GP_CMD_SYS_INSTALL_SCRIPT, parameters);
}
