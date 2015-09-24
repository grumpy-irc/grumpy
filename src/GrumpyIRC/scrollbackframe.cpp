//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "scrollbackframe.h"
#include "inputbox.h"
#include "ui_scrollbackframe.h"

using namespace GrumpyIRC;

ScrollbackFrame::ScrollbackFrame(ScrollbackFrame *parentWindow, QWidget *parent) : QFrame(parent), ui(new Ui::ScrollbackFrame)
{
    this->ui->setupUi(this);
    this->inputBox = new InputBox(this);
    this->ui->splitter->addWidget(this->inputBox);
    this->_parent = parentWindow;
}

ScrollbackFrame::~ScrollbackFrame()
{
    delete this->ui;
}

QString ScrollbackFrame::GetWindowName() const
{
    return this->_name;
}

void ScrollbackFrame::InsertText(ScrollbackItem item)
{
    this->buffer += item.GetTime().toString() + ": " + item.GetText() + "<br>";
    this->ui->textEdit->setHtml(buffer);
    Scrollback::InsertText(item);
}

void ScrollbackFrame::SetWindowName(QString title)
{
    this->_name = title;
}

void ScrollbackFrame::InsertChild(ScrollbackFrame *child)
{
    this->childItems.append(child);
}

ScrollbackFrame *ScrollbackFrame::Child(int row)
{
    return this->childItems.at(row);
}

int ScrollbackFrame::ChildCount() const
{
    return this->childItems.count();
}

int ScrollbackFrame::ModelRow() const
{
    if (this->_parent)
        return this->_parent->childItems.indexOf(const_cast<ScrollbackFrame*>(this));

    return 0;
}

ScrollbackFrame *ScrollbackFrame::GetParent()
{
    return this->_parent;
}

void ScrollbackFrame::SetParent(ScrollbackFrame *parentWindow)
{
    this->_parent = parentWindow;
}
