//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SCROLLBACKLIST_ITEMMODEL_H
#define SCROLLBACKLIST_ITEMMODEL_H

#include <QAbstractItemModel>

namespace GrumpyIRC
{
    class ScrollbackFrame;

    /*!
     * \brief The ScrollbackList_ItemModel class makes it super easy to organize windows in tree view
     */
    class ScrollbackList_ItemModel : public QAbstractItemModel
    {
        public:
            explicit ScrollbackList_ItemModel(QObject *parent = 0);
            ~ScrollbackList_ItemModel();
            QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
            Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
            QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
            QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
            QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
            int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
            int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
            ScrollbackFrame *GetRoot();

        private:
            void setupModelData();
            ScrollbackFrame *rootItem;
    };
}

#endif // SCROLLBACKLIST_ITEMMODEL_H
