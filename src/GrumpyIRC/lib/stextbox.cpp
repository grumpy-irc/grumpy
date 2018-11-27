//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "stextbox.h"
#include <libcore/exception.h>
#include <libcore/profiler.h>

using namespace GrumpyIRC;

STextBox::STextBox(QWidget *parent) : QPlainTextEdit(parent), GrumpyObject("STextBox")
{
    this->setReadOnly(true);
    this->setUndoRedoEnabled(false);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
}

STextBox::~STextBox()
{

}

void STextBox::AppendHtml(QString html)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    this->appendHtml(html);
}

void STextBox::SetStyleSheet(QString css)
{
    this->setStyleSheet(css);
}

void STextBox::Clear()
{
    this->clear();
}

bool STextBox::SendEvent(QEvent *e)
{
    return QPlainTextEdit::event(e);
}

void STextBox::dropEvent(QDropEvent *e)
{
    // do nothing!
}

void STextBox::mousePressEvent(QMouseEvent *e)
{
    this->clickedAnchor = (e->button() & Qt::LeftButton) ? anchorAt(e->pos()) : QString();
    QPlainTextEdit::mousePressEvent(e);
}

void STextBox::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() & Qt::LeftButton && !this->clickedAnchor.isEmpty() && anchorAt(e->pos()) == this->clickedAnchor)
    {
        emit this->Event_Link(this->clickedAnchor);
    }

    QPlainTextEdit::mouseReleaseEvent(e);
}

