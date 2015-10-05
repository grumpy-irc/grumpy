//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "skin.h"
#include "scrollbackframe.h"
#include "inputbox.h"
#include "ui_scrollbackframe.h"

using namespace GrumpyIRC;

ScrollbackFrame::ScrollbackFrame(ScrollbackFrame *parentWindow, QWidget *parent, Scrollback *_scrollback) : QFrame(parent), ui(new Ui::ScrollbackFrame)
{
    this->ui->setupUi(this);
    this->inputBox = new InputBox(this);
    this->ui->splitter->addWidget(this->inputBox);
    this->ui->textEdit->setPalette(Skin::Default->Palette());
    this->_parent = parentWindow;
    if (_scrollback == NULL)
        this->scrollback = new Scrollback();
    else
        this->scrollback = _scrollback;
    connect(this->scrollback, SIGNAL(Event_InsertText(ScrollbackItem)), this, SLOT(_insertText_(ScrollbackItem)));
}

ScrollbackFrame::~ScrollbackFrame()
{
    delete this->scrollback;
	//! \todo Handle deletion of TreeNode from list of scbs
	//delete this->TreeNode;
    delete this->ui;
}

QString ScrollbackFrame::GetWindowName() const
{
    return this->_name;
}

void ScrollbackFrame::InsertText(ScrollbackItem item)
{
    this->scrollback->InsertText(item);
}

void ScrollbackFrame::_insertText_(ScrollbackItem item)
{
    this->buffer += item.GetTime().toString() + ": " + item.GetText() + "<br>";
    this->ui->textEdit->setHtml(buffer);
}

void ScrollbackFrame::SetWindowName(QString title)
{
    this->_name = title;
}

ScrollbackFrame *ScrollbackFrame::GetParent()
{
    return this->_parent;
}

unsigned long ScrollbackFrame::GetID()
{
    return this->scrollback->GetID();
}

IRCSession *ScrollbackFrame::GetSession()
{
    return this->scrollback->GetSession();
}

Scrollback *ScrollbackFrame::GetScrollback()
{
    return this->scrollback;
}

void ScrollbackFrame::Focus()
{
    this->inputBox->Focus();
}

