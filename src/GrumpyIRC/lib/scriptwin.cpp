//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include <libcore/networksession.h>
#include <libcore/core.h>
#include "corewrapper.h"
#include <libcore/exception.h>
#include "scrollbackframe.h"
#include <libcore/commandprocessor.h>
#include "scriptwin.h"
#include "skin.h"
#include "ui_scriptwin.h"

using namespace GrumpyIRC;

ScriptWin::ScriptWin(ScrollbackFrame *parent) : QDialog(parent), ui(new Ui::ScriptWin)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->ui->setupUi(this);
    this->parentFrame = parent;
    this->ui->plainTextEdit->setFont(Skin::GetCurrent()->TextFont);
    this->ui->plainTextEdit->setPalette(Skin::GetCurrent()->Palette());
}

ScriptWin::~ScriptWin()
{
    delete this->ui;
}

void ScriptWin::Set(QString input)
{
    this->ui->plainTextEdit->setPlainText(input);
    this->ui->pushButton->setFocus();
}

void GrumpyIRC::ScriptWin::on_pushButton_clicked()
{
    QStringList ls = this->ui->plainTextEdit->toPlainText().split("\n");
    foreach (QString line, ls)
    {
        if (line.startsWith("#") || line.isEmpty())
            continue;

        if (line.startsWith(CoreWrapper::GrumpyCore->GetCommandProcessor()->CommandPrefix))
        {
            // This is a command
            CoreWrapper::GrumpyCore->GetCommandProcessor()->ProcessText(line, this->parentFrame->GetScrollback());
            continue;
        }

        // Send as a raw command
        if (!this->parentFrame->GetSession())
            throw new NullPointerException("this->parentFrame->GetSession()", BOOST_CURRENT_FUNCTION);

        this->parentFrame->GetSession()->SendRaw(this->parentFrame->GetScrollback(), line);
    }
    this->close();
}
