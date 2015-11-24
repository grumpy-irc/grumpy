//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SCROLLBACKLIST_NODE_H
#define SCROLLBACKLIST_NODE_H

#include <QStandardItemModel>
#include <QBrush>
#include <QIcon>

namespace GrumpyIRC
{
	class ScrollbackFrame;
	class ScrollbackList_Node : public QStandardItem
	{
		public:
			ScrollbackList_Node(ScrollbackFrame *sb);
			ScrollbackFrame *GetScrollback();
            void RebuildCache();
            void UpdateIcon();
            void UpdateToolTip();
            void UpdateColor();
            bool lowerThan(const QStandardItem &other) const;
            bool operator<(const QStandardItem &other) const;
            bool IsSystem = false;

		private:
            QBrush standardBrush;
            QBrush highlighterBrush;
            QBrush unreadBrush;
            QBrush systemBrush;
			ScrollbackFrame *scrollback;

	};
}

#endif // SCROLLBACKLIST_NODE_H
