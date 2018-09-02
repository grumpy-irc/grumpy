//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "uiscript.h"
#include "grumpyuijs.h"
#include "../grumpyeventhandler.h"
#include "../scrollbackframe.h"
#include "../inputbox.h"
#include "../scrollbacksmanager.h"
#include "../messagebox.h"
#include <libcore/core.h>
#include <libcore/scrollback.h>

using namespace GrumpyIRC;

UiScript::UiScript() : ScriptExtension()
{

}

QString UiScript::GetContext()
{
    return "GrumpyChat";
}

unsigned int UiScript::GetContextID()
{
    return GRUMPY_SCRIPT_CONTEXT_GRUMPY_CHAT;
}

void UiScript::Hook_OnExit()
{
    if (this->attachedHooks.contains(GRUMPY_SCRIPT_HOOK_UI_EXIT))
        this->executeFunction(this->attachedHooks[GRUMPY_SCRIPT_HOOK_UI_EXIT]);
}

void UiScript::Hook_OnHistory(Scrollback *window, QString text)
{
    if (!this->attachedHooks.contains(GRUMPY_SCRIPT_HOOK_UI_HISTORY))
        return;
    QJSValueList parameters;
    parameters.append(QJSValue(window->GetID()));
    parameters.append(QJSValue(text));
    this->executeFunction(this->attachedHooks[GRUMPY_SCRIPT_HOOK_UI_HISTORY], parameters);
}

void UiScript::Hook_WindowSwitch(Scrollback *window)
{
    if (!this->attachedHooks.contains(GRUMPY_SCRIPT_HOOK_UI_WINDOW_SWITCH))
        return;
    QJSValueList parameters;
    parameters.append(QJSValue(window->GetID()));
    this->executeFunction(this->attachedHooks[GRUMPY_SCRIPT_HOOK_UI_WINDOW_SWITCH], parameters);
}

void UiScript::Hook_OnMainWindowStart()
{
    if (!this->attachedHooks.contains(GRUMPY_SCRIPT_HOOK_UI_MAIN_LOAD))
        return;
    this->executeFunction(this->attachedHooks[GRUMPY_SCRIPT_HOOK_UI_MAIN_LOAD]);
}

void UiScript::Hook_OnScrollbackFrameCreated(Scrollback *window)
{
    if (!this->attachedHooks.contains(GRUMPY_SCRIPT_HOOK_UI_SCROLLBACK_CREATED))
        return;
    QJSValueList parameters;
    parameters.append(QJSValue(window->GetID()));
    this->executeFunction(this->attachedHooks[GRUMPY_SCRIPT_HOOK_UI_SCROLLBACK_CREATED], parameters);
}

int UiScript::GetHookID(QString hook)
{
    if (hook == "ui_main")
        return GRUMPY_SCRIPT_HOOK_UI_MAIN_LOAD;
    if (hook == "ui_scrollback_frame_created")
        return GRUMPY_SCRIPT_HOOK_UI_SCROLLBACK_CREATED;
    if (hook == "ui_exit")
        return GRUMPY_SCRIPT_HOOK_UI_EXIT;
    if (hook == "ui_history")
        return GRUMPY_SCRIPT_HOOK_UI_HISTORY;
    if (hook == "ui_window_switch")
        return GRUMPY_SCRIPT_HOOK_UI_WINDOW_SWITCH;

    return ScriptExtension::GetHookID(hook);
}

void UiScript::registerFunctions()
{
    ScriptExtension::registerFunctions();
}

void UiScript::registerClasses()
{
    this->registerClass("grumpy_ui", new GrumpyUIJS(this));
    ScriptExtension::registerClasses();
}
