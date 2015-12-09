//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "hooks.h"
#include "../libcore/scrollback.h"
#include "scrollbackframe.h"
#include "mainwindow.h"

using namespace GrumpyIRC;

void UiHooks::OnScrollbackItemHighlight(ScrollbackFrame *scrollback, ScrollbackItem *item)
{
    if (!scrollback->IsVisible())
        scrollback->GetScrollback()->SetState(ScrollbackState_UnreadNotice);
    // Triggered when any item is highlighted by system
    if (!scrollback->IsVisible() || !MainWindow::Main->isActiveWindow())
    {
        if (item->GetUser().GetNick().isEmpty() == false)
            MainWindow::Main->Notify(scrollback->GetTitle(), item->GetUser().GetNick() + ": " + item->GetText());
        else
            MainWindow::Main->Notify(scrollback->GetTitle(), item->GetText());
    }
}


