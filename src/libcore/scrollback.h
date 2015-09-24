//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SCROLLBACK_H
#define SCROLLBACK_H

#include "libcore_global.h"
#include <QString>
#include <QDateTime>
#include <QMutex>
#include <QList>

namespace GrumpyIRC
{
    enum ScrollbackItemType
    {
        ScrollbackItemType_Unknown,
        ScrollbackItemType_Message,
        ScrollbackItemType_Join,
        ScrollbackItemType_Part,
        ScrollbackItemType_Kick,
        ScrollbackItemType_Quit,
        ScrollbackItemType_Act,
        ScrollbackItemType_System
    };

    /*!
     * \brief The ScrollbackItem class is a one item in scrollback buffer
     */
    class LIBCORESHARED_EXPORT ScrollbackItem
    {
        public:
            ScrollbackItem(QString text);
            virtual ~ScrollbackItem();
            virtual QString GetText() const;
            virtual ScrollbackItemType GetType() const;
            virtual QDateTime GetTime() const;
            virtual void SetType(ScrollbackItemType type);
            virtual void SetText(QString text);
        private:
            QString _text;
            QDateTime _datetime;
            ScrollbackItemType _type;
    };

    /*!
     * \brief The Scrollback class represent a buffer used to store all items in a window
     */
    class LIBCORESHARED_EXPORT Scrollback
    {
        public:
            static QList<Scrollback*> ScrollbackList;
            static QMutex ScrollbackList_Mutex;

            Scrollback();
            virtual ~Scrollback();
            unsigned long long GetMaxItemsSize();
            unsigned long long GetID();
            void SetMaxItemsSize(unsigned long long size);
            virtual void InsertText(QString text, ScrollbackItemType type = ScrollbackItemType_System);
            virtual void InsertText(ScrollbackItem item);

        private:
            static unsigned long long lastID;
            QList<ScrollbackItem> items;
            unsigned long long _id;
            unsigned long long _maxItems;
    };
}

#endif // SCROLLBACK_H
