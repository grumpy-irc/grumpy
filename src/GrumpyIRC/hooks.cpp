//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "hooks.h"
#include "uiscript.h"
#include "../libcore/core.h"
#include "../libcore/scrollback.h"
#include "scrollbackframe.h"
#include "mainwindow.h"

using namespace GrumpyIRC;

void UiHooks::OnExit()
{
    foreach (ScriptExtension *script, UiScript::GetExtensions())
    {
        if (script->GetContextID() == GRUMPY_SCRIPT_CONTEXT_GRUMPY_CHAT)
            ((UiScript*)script)->Hook_OnExit();
    }
}

void UiHooks::OnMainWindowStart()
{
    foreach (ScriptExtension *script, UiScript::GetExtensions())
    {
        if (script->GetContextID() == GRUMPY_SCRIPT_CONTEXT_GRUMPY_CHAT)
            ((UiScript*)script)->Hook_OnMainWindowStart();
    }
}

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

void UiHooks::OnNewScrollbackFrame(ScrollbackFrame *scrollback)
{
    foreach (ScriptExtension *script, UiScript::GetExtensions())
    {
        if (script->GetContextID() == GRUMPY_SCRIPT_CONTEXT_GRUMPY_CHAT)
            ((UiScript*)script)->Hook_OnScrollbackFrameCreated(scrollback->GetScrollback());
    }
}

void UiHooks::OnInput()
{
    MainWindow::Main->ResetAutoAway();
}

void UiHooks::OnInputHistInsert(ScrollbackFrame *scrollback, QString text)
{
    foreach (ScriptExtension *script, UiScript::GetExtensions())
    {
        if (script->GetContextID() == GRUMPY_SCRIPT_CONTEXT_GRUMPY_CHAT)
            ((UiScript*)script)->Hook_OnHistory(scrollback->GetScrollback(), text);
    }
}


