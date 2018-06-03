//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "messagebox.h"
#include "grumpyconf.h"
#include "scriptform.h"
#include "uiscript.h"
#include "ui_scriptform.h"
#include "jshighlighter.h"
#include <libcore/configuration.h>
#include <libcore/core.h>
#include <libcore/scriptextension.h>
#include <libcore/resources.h>
#include <QFontDatabase>
#include <QHash>
#include <QFile>
#include <QFileDialog>

using namespace GrumpyIRC;

ScriptForm::ScriptForm(QWidget *parent) : QDialog(parent), ui(new Ui::ScriptForm)
{
    this->ui->setupUi(this);
    this->ui->textEdit->setText(Resources::GetSource("/grumpy_core/ecma/template"));
    this->ui->textEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    QString file_name = Core::GrumpyCore->GetConfiguration()->GetHomeScriptPath() + "new_script.js";
    int file_id = 1;
    while (QFile(file_name).exists())
    {
        file_name = Core::GrumpyCore->GetConfiguration()->GetHomeScriptPath() + "new_script" + QString::number(file_id++) + ".js";
    }
    this->ui->lineEdit_2->setText(file_name);
    this->highlighter = new JSHighlighter(this->ui->textEdit->document());
}

ScriptForm::~ScriptForm()
{
    delete this->highlighter;
    delete this->ui;
}

void ScriptForm::EditScript(QString path, QString script_name)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadWrite))
    {
        MessageBox::Error("Error", "Unable to open " + path + " for writing", this);
        return;
    }
    this->ui->textEdit->setText(QString(file.readAll()));
    this->editing = true;
    this->editingName = script_name;
    this->ui->lineEdit_2->setText(path);
    this->ui->pushButton->setText("Save, reload and close");
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
    if (!path.isEmpty())
        this->ui->lineEdit_2->setText(path);
}

void ScriptForm::on_pushButton_clicked()
{
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
