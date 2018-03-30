//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "../libcore/autocompletionengine.h"
#include "../libcore/commandprocessor.h"
#include "corewrapper.h"
#include "../libcore/core.h"
#include "inputbox.h"
#include "hooks.h"
#include "messagebox.h"
#include "skin.h"
#include "keyfilter.h"
#include "ui_inputbox.h"
#include "scrollbackframe.h"

using namespace GrumpyIRC;

AutocompletionEngine *InputBox::AE = NULL;

InputBox::InputBox(ScrollbackFrame *parent) : QFrame(parent), ui(new Ui::InputBox)
{
    this->ui->setupUi(this);
    this->ui->lineEdit->setVisible(false);
    this->ui->textEdit->installEventFilter(new KeyFilter(this));
    this->historySize = 800;
    this->ui->textEdit->setFont(Skin::GetCurrent()->TextFont);
    this->ui->textEdit->setPalette(Skin::GetCurrent()->Palette());
    this->ui->lineEdit->setPalette(Skin::GetCurrent()->Palette());
    this->historyPosition = 0;
    this->isSecure = false;
    this->parent = parent;
}

InputBox::~InputBox()
{
    delete this->ui;
}

void InputBox::ProcessInput()
{
    UiHooks::OnInput();
    // Check if the text user is about to send isn't too long
    QString text = this->ui->textEdit->toPlainText();
    int size = text.size();
    if (size > 800 && !text.trimmed().startsWith(CoreWrapper::GrumpyCore->GetCommandProcessor()->CommandPrefix))
    {
        MessageBoxResponse response = MessageBox::Question("really-send-long-text", "Text is long", "Text you input is " + QString::number(size) + " letters long. Do you really want to send it? Please note that on some channels you may get banned for flooding.");
        if (response != MessageBoxResponse_Yes)
            return;
    }
    CoreWrapper::GrumpyCore->GetCommandProcessor()->ProcessText(text, this->parent->GetScrollback());
    this->insertToHistory();
    this->historyPosition = this->history.count();
    this->ui->textEdit->setText("");
}

void InputBox::Secure()
{
    this->isSecure = !this->isSecure;
    this->ui->lineEdit->setVisible(this->isSecure);
    this->ui->textEdit->setVisible(!this->isSecure);
    if (this->isSecure)
    {
        // Copy the text from textEdit to lineEdit
        this->ui->lineEdit->setText(this->ui->textEdit->toPlainText());
        this->ui->lineEdit->setCursorPosition(this->ui->textEdit->textCursor().position());
        this->ui->lineEdit->setFocus();
        this->ui->textEdit->setText("");
    } else
    {
        this->ui->textEdit->setFocus();
        this->ui->lineEdit->setText("");
        this->ui->textEdit->setText("");
    }
}

void InputBox::Complete()
{
    if (this->isSecure)
        return;

    if (!InputBox::AE)
        return;

    AutocompletionInformation input;
    QList<QString> commands;
    if (this->parent->IsConnectedToIRC())
    {
        // these are typical IRC commands supported by majority of servers
        commands << "join"
            << "part"
            << "kick"
            << "mode"
            << "whois"
            << "whowas"
            << "who"
            << "topic"
            << "knock"
            << "sajoin"
            << "samode";
    }
    input.Position = this->ui->textEdit->textCursor().position();
    input.FullText = this->ui->textEdit->toPlainText();

    AutocompletionInformation result = AE->Execute(input, commands, this->parent->GetUsers(), this->parent->GetChannels());
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

void InputBox::ClearHistory()
{
    this->historyPosition = 0;
    this->history.clear();
}

void InputBox::LoadHistory(QList<QString> new_history)
{
    foreach (QString item, new_history)
        this->history.append(item);
    while ((unsigned int)this->history.size() > this->historySize)
        this->history.removeAt(0);

    this->historyPosition = this->history.count();
}

void InputBox::Focus()
{
    this->ui->textEdit->setFocus();
}

void InputBox::InsertAtCurrentPosition(QString text)
{
    if (!this->isSecure)
        this->ui->textEdit->insertPlainText(text);
}

void InputBox::InsertEnter()
{
    this->ui->textEdit->insertPlainText("\n");
}

bool InputBox::IsSecure()
{
    return this->isSecure;
}

void InputBox::History(bool up)
{
    if (this->isSecure)
        return;

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
    QTextCursor cursor = this->ui->textEdit->textCursor();
    cursor.setPosition(this->ui->textEdit->toPlainText().size());
    this->ui->textEdit->setTextCursor(cursor);
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
        UiHooks::OnInputHistInsert(this->parent, line);
        this->history.append(line);
    }
}

void GrumpyIRC::InputBox::on_lineEdit_returnPressed()
{
    UiHooks::OnInput();
    CoreWrapper::GrumpyCore->GetCommandProcessor()->ProcessText(this->ui->lineEdit->text(), this->parent->GetScrollback());
    this->ui->lineEdit->setText("");
}
