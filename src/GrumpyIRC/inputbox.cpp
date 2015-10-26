//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "../libcore/autocompletionengine.h"
#include "../libcore/commandprocessor.h"
#include "corewrapper.h"
#include "../libcore/core.h"
#include "inputbox.h"
#include "skin.h"
#include "keyfilter.h"
#include "ui_inputbox.h"
#include "scrollbackframe.h"

using namespace GrumpyIRC;

AutocompletionEngine *InputBox::AE = NULL;

InputBox::InputBox(ScrollbackFrame *parent) : QFrame(parent), ui(new Ui::InputBox)
{
    this->ui->setupUi(this);
    this->ui->textEdit->setPalette(Skin::Default->Palette());
    this->ui->textEdit->installEventFilter(new KeyFilter(this));
    this->historySize = 800;
    this->historyPosition = 0;
    this->parent = parent;
}

InputBox::~InputBox()
{
    delete this->ui;
}

void InputBox::ProcessInput()
{
    CoreWrapper::GrumpyCore->GetCommandProcessor()->ProcessText(this->ui->textEdit->toPlainText(), this->parent->GetScrollback());
    this->insertToHistory();
    this->historyPosition = this->history.count();
    this->ui->textEdit->setText("");
}

void InputBox::Complete()
{
    if (!InputBox::AE)
        return;

    AutocompletionInformation input;
    input.Position = this->ui->textEdit->textCursor().position();
    input.FullText = this->ui->textEdit->toPlainText();

    AutocompletionInformation result = AE->Execute(input);
    if (!result.Success)
        return;
    this->ui->textEdit->setText(result.FullText);
    QTextCursor cursor = this->ui->textEdit->textCursor();
    cursor.setPosition(result.Position);
    this->ui->textEdit->setTextCursor(cursor);
    if (result.Suggestions.count())
    {
        QString suggestions;
        foreach(QString sx, result.Suggestions)
            suggestions += sx + ", ";
        if (suggestions.endsWith(", "))
            suggestions = suggestions.mid(0, suggestions.size() - 2);
        this->parent->InsertText(QString("Multiple results: ") + suggestions);
    }
}

void InputBox::Focus()
{
    this->ui->textEdit->setFocus();
}

void InputBox::History(bool up)
{
    if (up)
    {
        if (this->historyPosition <= 0)
            return;
        if (!this->history.contains(this->ui->textEdit->toPlainText()))
        {
            this->insertToHistory();
        }
        this->ui->textEdit->setText(this->history[--this->historyPosition]);
    } else
    {
        if (this->historyPosition >= (this->history.count()-1))
            return;
        this->ui->textEdit->setText(this->history[++this->historyPosition]);
    }
}

void InputBox::insertToHistory()
{
    foreach (QString line, this->ui->textEdit->toPlainText().split("\n"))
    {
        if (line.trimmed().isEmpty())
            continue;
        while ((unsigned int)this->history.size() > this->historySize)
        {
            this->history.removeAt(0);
        }
        this->history.append(line);
    }
}
