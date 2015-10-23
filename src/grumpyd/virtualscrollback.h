//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef VIRTUALSCROLLBACK_H
#define VIRTUALSCROLLBACK_H

#include "../libcore/scrollback.h"

namespace GrumpyIRC
{
    class User;

    /*!
     * \brief The VirtualScrollback class is used to add extra networking layer to standard scrollbacks
     *        thanks to that we can send multiplexed information about new lines written to each
     *        scrollback directly to every logged session for each user this scrollback belongs to
     */
    class VirtualScrollback : public Scrollback
    {
            Q_OBJECT
        public:
            VirtualScrollback(ScrollbackType Type = ScrollbackType_System, Scrollback *parent = NULL);
            ~VirtualScrollback();
            User *GetOwner() const;
            void Sync();
            void SetOwner(User *user);
            void InsertText(ScrollbackItem item);

        private:
            User *owner;
    };
}

#endif // VIRTUALSCROLLBACK_H
