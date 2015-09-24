//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "scrollbacklist_itemmodel.h"
#include "scrollbackframe.h"

using namespace GrumpyIRC;

ScrollbackList_ItemModel::ScrollbackList_ItemModel(QObject *parent) : QAbstractItemModel(parent)
{
    // There doesn't really need to be any text, this is basically just a header which is hidden
    // it needs to exist because Qt seems to require that
    this->rootItem = new ScrollbackFrame();
    this->rootItem->SetWindowName("Windows");
    setupModelData();
}

ScrollbackList_ItemModel::~ScrollbackList_ItemModel()
{
    delete this->rootItem;
}

QVariant ScrollbackList_ItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ScrollbackFrame *item = static_cast<ScrollbackFrame*>(index.internalPointer());

    if (index.column() == 0)
        return QVariant(item->GetWindowName());
    if (index.column() == 1)
        return QVariant(item->GetID());

    return QVariant();
}

Qt::ItemFlags ScrollbackList_ItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return NULL;

    return QAbstractItemModel::flags(index);
}

QVariant ScrollbackList_ItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

QModelIndex ScrollbackList_ItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ScrollbackFrame *parentItem;

    if (!parent.isValid())
        parentItem = this->rootItem;
    else
        parentItem = static_cast<ScrollbackFrame*>(parent.internalPointer());

    ScrollbackFrame *childItem = parentItem->Child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ScrollbackList_ItemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ScrollbackFrame *childItem = static_cast<ScrollbackFrame*>(index.internalPointer());
    ScrollbackFrame *parentItem = childItem->GetParent();

    if (parentItem == this->rootItem)
        return QModelIndex();

    return createIndex(parentItem->ModelRow(), 0, parentItem);
}

int ScrollbackList_ItemModel::rowCount(const QModelIndex &parent) const
{
    ScrollbackFrame *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = this->rootItem;
    else
        parentItem = static_cast<ScrollbackFrame*>(parent.internalPointer());

    return parentItem->ChildCount();
}

int ScrollbackList_ItemModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

ScrollbackFrame *ScrollbackList_ItemModel::GetRoot()
{
    return this->rootItem;
}

void ScrollbackList_ItemModel::setupModelData()
{

}

