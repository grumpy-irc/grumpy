//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef GUIHOOKS_H
#define GUIHOOKS_H

#include "../libcore/definitions.h"

namespace GrumpyIRC
{
    class ScrollbackFrame;
    class ScrollbackItem;

    class UiHooks
    {
        public:
            static void OnScrollbackItemHighlight(ScrollbackFrame *scrollback, ScrollbackItem *item);
            //! Called anytime user generates some sort of user input - click a window, write into console, whatever
            static void OnInput();
    };
}

#endif // HOOKS_H
