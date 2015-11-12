//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QAction>
#include <QMenu>
#include <QDebug>
#include "mainwindow.h"
#include "scrollbacklist.h"
#include "../libcore/networksession.h"
#include "../libcore/ircsession.h"
#include "packetsnifferwin.h"
#include "skin.h"
#include "ui_scrollbacklist.h"
#include "../libcore/generic.h"
#include "scrollbackframe.h"
#include "scrollbacksmanager.h"
#include "scrollbacklist_node.h"

using namespace GrumpyIRC;

ScrollbackList::ScrollbackList(QWidget *parent) : QDockWidget(parent), ui(new Ui::ScrollbackList)
{
    this->ui->setupUi(this);
    this->ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    this->model = new QStandardItemModel(0, 2, this);
    this->root = this->model->invisibleRootItem();
    this->ui->treeView->setModel(this->model);
    this->ui->treeView->setColumnHidden(1, true);
    this->ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->treeView->setHeaderHidden(true);
    this->ui->treeView->setPalette(Skin::GetDefault()->Palette());
}

ScrollbackList::~ScrollbackList()
{
    delete this->ui;
}

void ScrollbackList::RegisterWindow(ScrollbackFrame *scrollback, ScrollbackList_Node *parent_node)
{
    QStandardItem *root;
    if (parent_node != NULL)
        root = parent_node;
    else
        root = this->GetRootTreeItem();
    ScrollbackList_Node *node = new ScrollbackList_Node(scrollback);
    scrollback->TreeNode = node;
    root->appendRow(node);
    this->ui->treeView->expand(root->index());
}

QStandardItem *ScrollbackList::GetRootTreeItem()
{
    return this->root;
}

void ScrollbackList::UnregisterWindow(ScrollbackList_Node *node, ScrollbackList_Node *parent_n)
{
    QModelIndex index = this->model->indexFromItem(node);
    QModelIndex parent;
    if (parent_n != NULL)
        parent = this->model->indexFromItem(parent_n);
    this->model->removeRow(index.row(), parent);
}

void GrumpyIRC::ScrollbackList::on_treeView_activated(const QModelIndex &index)
{
    this->switchWindow(index);
}

void GrumpyIRC::ScrollbackList::on_treeView_customContextMenuRequested(const QPoint &pos)
{
    ScrollbackFrame *wx = this->selectedWindow();
    if (!wx)
        return;

    // We have some window selected so let's display a menu
    QPoint globalPos = this->ui->treeView->viewport()->mapToGlobal(pos);
    QMenu Menu;
    // Items
    QAction *menuClose = new QAction(QObject::tr("Close"), &Menu);
    Menu.addAction(menuClose);
    QAction *menuInsrFavorites = NULL;
    QAction *menuAuto = NULL;
    QAction *menuPart = NULL;
    QAction *menuDisconnect = NULL;
    QAction *menuSniffer = NULL;

    if (!wx->IsDeletable || !wx->IsDead())
        menuClose->setEnabled(false);

    if (wx->IsNetwork())
    {
        menuInsrFavorites = new QAction("Insert to favorite networks", &Menu);
        Menu.addAction(menuInsrFavorites);
        menuAuto = new QAction("Automatically connect", &Menu);
        menuAuto->setCheckable(true);
        menuAuto->setChecked(false);
        Menu.addAction(menuAuto);
    }

    if (wx->IsChannel())
    {
        menuPart = new QAction(QObject::tr("Part"), &Menu);
        // Check if we can part the chan
        if (wx->IsDead())
            menuPart->setEnabled(false);
        Menu.addAction(menuPart);
    }
    if (wx->IsNetwork())
    {
        menuDisconnect = new QAction(QObject::tr("Disconnect"), &Menu);
        Menu.addAction(menuDisconnect);
        menuSniffer = new QAction(QObject::tr("Network sniffer"), &Menu);
        Menu.addAction(menuSniffer);
    }

    QAction* selectedItem = Menu.exec(globalPos);
    if (!selectedItem)
        return;
    if (selectedItem == menuClose)
    {
        this->closeWindow();
    } else if (selectedItem == menuPart)
    {
        wx->RequestPart();
    } else if (selectedItem == menuDisconnect)
    {
        wx->RequestDisconnect();
    } else if (selectedItem == menuSniffer)
    {
        this->sniffer(wx);
    }
}

void ScrollbackList::switchWindow(const QModelIndex &index)
{
    if (this->model->itemFromIndex(index) == this->root)
        return;
    ScrollbackList_Node *node = (ScrollbackList_Node*)this->model->itemFromIndex(index);
    if (!node)
        return;
    MainWindow::Main->GetScrollbackManager()->SwitchWindow(node->GetScrollback());
}

void ScrollbackList::sniffer(ScrollbackFrame *window)
{
    NetworkSession *session = window->GetSession();
    if (!session)
        return;
    if (session->GetType() == SessionType_IRC)
    {
        PacketSnifferWin *wx = new PacketSnifferWin();
        wx->setAttribute(Qt::WA_DeleteOnClose);
        wx->Load((IRCSession*)session);
        wx->show();
    }
    else
    {
        Generic::MessageBox("Unsupported", "This protocol doesn't support this feature", Generic::MessageBox_Type_Error);
    }
}

void ScrollbackList::closeWindow()
{
    QModelIndex index = this->ui->treeView->currentIndex();
    ScrollbackList_Node *node = (ScrollbackList_Node*)this->model->itemFromIndex(index);
    if (!node)
        return;
    node->GetScrollback()->RequestClose();
}

ScrollbackFrame *ScrollbackList::selectedWindow()
{
    QModelIndex index = this->ui->treeView->currentIndex();
    ScrollbackList_Node *node = (ScrollbackList_Node*)this->model->itemFromIndex(index);
    if (node == 0)
        return NULL;
    return node->GetScrollback();
}

void GrumpyIRC::ScrollbackList::on_treeView_clicked(const QModelIndex &index)
{
    this->switchWindow(index);
}
