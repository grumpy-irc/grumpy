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
#include "ui_scrollbacklist.h"
#include "scrollbackframe.h"
#include "scrollbacksmanager.h"

using namespace GrumpyIRC;

ScrollbackList::ScrollbackList(QWidget *parent) : QDockWidget(parent), ui(new Ui::ScrollbackList)
{
    this->ui->setupUi(this);
    this->model = new ScrollbackList_ItemModel(this);
    this->ui->treeView->setModel(this->model);
    this->ui->treeView->setColumnHidden(1, true);
    this->ui->treeView->setHeaderHidden(true);
}

ScrollbackList::~ScrollbackList()
{
    delete this->ui;
}

void ScrollbackList::RegisterWindow(ScrollbackFrame *scrollback, ScrollbackFrame *parentWindow)
{
    ScrollbackFrame *root;
    if (parentWindow != NULL)
        root = parentWindow;
    else
        root = this->model->GetRoot();
    if (!scrollback->GetParent())
    {
        // Windows that were created before scrollbacklist are probably missing the parent, we can fix them, by giving tree root as parent to them
        scrollback->SetParent(this->model->GetRoot());
    }
    root->InsertChild(scrollback);
}

ScrollbackFrame *ScrollbackList::GetRootTreeItem()
{
    return this->model->GetRoot();
}

void GrumpyIRC::ScrollbackList::on_treeView_activated(const QModelIndex &index)
{
    QMap<int, QVariant> data = this->model->itemData(index);
    //qWarning() << data[0].toString();
    MainWindow::Main->GetScrollbackManager()->SwitchWindow(data[1].toULongLong());
}
