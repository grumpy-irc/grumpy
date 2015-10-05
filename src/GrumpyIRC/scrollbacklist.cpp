//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QDebug>
#include "mainwindow.h"
#include "scrollbacklist.h"
#include "skin.h"
#include "ui_scrollbacklist.h"
#include "scrollbackframe.h"
#include "scrollbacksmanager.h"
#include "scrollbacklist_node.h"

using namespace GrumpyIRC;

ScrollbackList::ScrollbackList(QWidget *parent) : QDockWidget(parent), ui(new Ui::ScrollbackList)
{
    this->ui->setupUi(this);
    this->model = new QStandardItemModel(0, 2, this);
	this->root = this->model->invisibleRootItem();
    this->ui->treeView->setModel(this->model);
    this->ui->treeView->setColumnHidden(1, true);
	this->ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->treeView->setHeaderHidden(true);
    this->ui->treeView->setPalette(Skin::Default->Palette());
}

ScrollbackList::~ScrollbackList()
{
    delete this->ui;
}

void ScrollbackList::RegisterWindow(ScrollbackFrame *scrollback, QStandardItem *parent_node)
{
    QStandardItem *root;
	if (parent_node != NULL)
		root = parent_node;
    else
        root = this->GetRootTreeItem();
	QStandardItem *node = new ScrollbackList_Node(scrollback);
	scrollback->TreeNode = node;
    root->appendRow(node);
}

QStandardItem *ScrollbackList::GetRootTreeItem()
{
    return this->root;
}

void GrumpyIRC::ScrollbackList::on_treeView_activated(const QModelIndex &index)
{
	if (this->model->itemFromIndex(index) == this->root)
		return;
	ScrollbackList_Node *node = (ScrollbackList_Node*)this->model->itemFromIndex(index);
	if (!node)
		return;
    MainWindow::Main->GetScrollbackManager()->SwitchWindow(node->GetScrollback());
}

void GrumpyIRC::ScrollbackList::on_treeView_customContextMenuRequested(const QPoint &pos)
{

}
