//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "scrollbacksmanager.h"
#include "grumpyconf.h"
#include "scrollbacklist.h"
#include "scrollbackframe.h"
#include "hooks.h"
#include "userframe.h"
#include "userwidget.h"
#include "mainwindow.h"
#include "../libcore/networksession.h"
#include "../libirc/libircclient/channel.h"
#include "../libcore/exception.h"
#include "ui_scrollbacksmanager.h"

using namespace GrumpyIRC;

ScrollbacksManager *ScrollbacksManager::Global = NULL;

ScrollbacksManager::ScrollbacksManager(QWidget *parent) : QFrame(parent), ui(new Ui::ScrollbacksManager)
{
    this->currentWidget = NULL;
    this->ui->setupUi(this);
}

ScrollbacksManager::~ScrollbacksManager()
{
    qDeleteAll(this->Scrollbacks);
    this->Scrollbacks.clear();
    delete this->ui;
}

ScrollbackFrame *ScrollbacksManager::CreateWindow(QString name, ScrollbackFrame *parent, bool focus, bool is_deletable, Scrollback *scrollback, bool is_system)
{
    ScrollbackFrame *window = new ScrollbackFrame(parent, NULL, scrollback, is_system);
    window->SetWindowName(name);
    this->Scrollbacks.append(window);
    window->IsDeletable = is_deletable;
    if (focus)
        this->SwitchWindow(window);
    ScrollbackList *scrollbacks = MainWindow::Main->GetScrollbackList();
    ScrollbackList_Node *parent_tree = NULL;
    if (parent)
        parent_tree = parent->TreeNode;
    if (scrollbacks)
        scrollbacks->RegisterWindow(window, parent_tree);
    UiHooks::OnNewScrollbackFrame(window);
    return window;
}

ScrollbackFrame *ScrollbacksManager::GetWindowFromID(scrollback_id_t id)
{
    foreach (ScrollbackFrame *sb, this->Scrollbacks)
    {
        if (sb->GetID() == id)
            return sb;
    }

    return NULL;
}

ScrollbackFrame *ScrollbacksManager::GetWindowFromScrollback(Scrollback *scrollback)
{
    if (scrollback == NULL)
        return NULL;
    foreach (ScrollbackFrame *xx, this->Scrollbacks)
    {
        if (xx->GetScrollback() == scrollback)
            return xx;
    }
    return NULL;
}

void ScrollbacksManager::DestroyWindow(ScrollbackFrame *window)
{
    if (!window->IsDeletable)
        return;
    this->SwitchWindow(MainWindow::Main->GetSystem());
    if (window->TreeNode)
    {
        ScrollbackList_Node *parent = NULL;
        if (window->GetParent())
            parent = window->GetParent()->TreeNode;
        ScrollbackList::GetScrollbackList()->UnregisterWindow(window->TreeNode, parent);
        window->TreeNode = NULL;
    }
    this->Scrollbacks.removeOne(window);
    delete window;
}

void ScrollbacksManager::SwitchWindow(scrollback_id_t id)
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
    {
        this->currentWidget->SetVisible(false);
        this->currentWidget->EnableState(true);
        QLayoutItem *container = this->ui->verticalLayout_2->itemAt(0);
        this->ui->verticalLayout_2->removeItem(this->ui->verticalLayout_2->itemAt(0));
        this->ui->verticalLayout_2->removeWidget(this->currentWidget);
        this->currentWidget->hide();
        this->ui->verticalLayout_2->addWidget(window);
        window->show();
        delete container;
    }
    else
    {
        this->ui->verticalLayout_2->addWidget(window);
    }

    // Switch the user widget
    UserWidget *usrw = MainWindow::Main->GetUsers();
    usrw->SetFrame(window->GetUserFrame());
    if (window->GetScrollback()->GetType() == ScrollbackType_Channel)
    {
        usrw->show();
    } else
    {
        // Hide the user widget
        usrw->hide();
    }
    this->currentWidget = window;
    this->currentWidget->Focus();

    // Redraw the buffer contents if needed
    QString name;
    switch (window->GetScrollback()->GetType())
    {
        case ScrollbackType_Channel:
            name = CONF->GetChannelHeader();
            break;
        case ScrollbackType_System:
        case ScrollbackType_User:
            name = CONF->GetLabeledHeader();
            break;
    }
    if (window->GetTitle().isEmpty())
    {
        name = "";
    } else
    {
        name.replace("$title", window->GetTitle());
        if (window->GetScrollback()->GetType() == ScrollbackType_Channel)
        {
            libircclient::Channel *channel = window->GetScrollback()->GetSession()->GetChannel(window->GetScrollback());
            if (channel != nullptr)
                name.replace("$topic", channel->GetTopic());
            else
                name.replace("$topic", " (dead)");
        }
    }
    MainWindow::Main->EnableGrumpydContext(window->IsGrumpy());
    MainWindow::Main->SetWN(name);

    window->EnableState(false);
    window->SetVisible(true);
    window->RefreshHtmlIfNeeded();
}

ScrollbackFrame *ScrollbacksManager::GetCurrentScrollback() const
{
    if (this->currentWidget == NULL)
        throw new GrumpyIRC::NullPointerException("this->currentWidget", BOOST_CURRENT_FUNCTION);

    // Return a currently selected window
    return this->currentWidget;
}
