//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef SCROLLBACKLIST_NODE_H
#define SCROLLBACKLIST_NODE_H

#include "grumpy_global.h"
#include <libcore/grumpyobject.h>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QBrush>
#include <QList>
#include <QIcon>

namespace GrumpyIRC
{
    class ScrollbackFrame;
    class LIBGRUMPYSHARED_EXPORT ScrollbackList_Node : public QStandardItem, public GrumpyObject
    {
        public:
            static QList<ScrollbackList_Node*> NodesList;

			ScrollbackList_Node(ScrollbackFrame *sb);
            ~ScrollbackList_Node();
			ScrollbackFrame *GetScrollback();
            void RebuildCache();
            void UpdateIcon();
            void UpdateToolTip();
            void UpdateColor();
            bool lowerThan(const QStandardItem &other) const;
            bool operator<(const QStandardItem &other) const;
            bool IsSystem;

		private:
            QBrush standardBrush;
            QBrush highlighterBrush;
            QBrush unreadBrush;
            QBrush systemBrush;
            ScrollbackFrame *scrollback;

	};
}

#endif // SCROLLBACKLIST_NODE_H
