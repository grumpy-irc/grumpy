//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "scrollbacksmanager.h"
#include "scrollbackframe.h"
#include "scrollbacklist.h"
#include "mainwindow.h"
#include "ui_scrollbacksmanager.h"

using namespace GrumpyIRC;

ScrollbacksManager::ScrollbacksManager(QWidget *parent) : QFrame(parent), ui(new Ui::ScrollbacksManager)
{
    this->ui->setupUi(this);
}

ScrollbacksManager::~ScrollbacksManager()
{
    qDeleteAll(this->Scrollbacks);
    this->Scrollbacks.clear();
    delete this->ui;
}

ScrollbackFrame *ScrollbacksManager::CreateWindow(QString name, ScrollbackFrame *parent, bool focus, bool is_deletable)
{
    ScrollbackFrame *window = new ScrollbackFrame(parent);
    window->SetWindowName(name);
    this->Scrollbacks.append(window);
    window->IsDeletable = is_deletable;
    if (focus)
        this->SwitchWindow(window);
    ScrollbackList *scrollbacks = MainWindow::Main->GetScrollbackList();
    if (scrollbacks)
        scrollbacks->RegisterWindow(window, parent);
    return window;
}

ScrollbackFrame *ScrollbacksManager::GetWindowFromID(unsigned long long id)
{
    foreach (ScrollbackFrame *sb, this->Scrollbacks)
    {
        if (sb->GetID() == id)
            return sb;
    }

    return NULL;
}

void ScrollbacksManager::DestroyWindow(ScrollbackFrame *window)
{
    if (!window->IsDeletable)
        return;
    this->Scrollbacks.removeOne(window);
    delete window;
}

void ScrollbacksManager::SwitchWindow(unsigned long long id)
{
    ScrollbackFrame *scrollback = this->GetWindowFromID(id);
    if (scrollback)
        this->SwitchWindow(scrollback);
}

void ScrollbacksManager::SwitchWindow(ScrollbackFrame *window)
{   
    if (this->currentWidget == window)
        return;

    if (this->currentWidget != NULL)
        this->ui->verticalLayout_2->removeWidget(this->currentWidget);

    this->currentWidget = window;
    this->ui->verticalLayout_2->addWidget(window);
}
