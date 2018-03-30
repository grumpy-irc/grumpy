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
#include "../libcore/scrollback.h"
#include <QtScript>

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
    this->executeFunction("ext_ui_on_exit");
}

void UiScript::Hook_OnHistory(Scrollback *window, QString text)
{
    QScriptValueList parameters;
    parameters.append(QScriptValue(this->engine, window->GetID()));
    parameters.append(QScriptValue(this->engine, text));
    this->executeFunction("ext_ui_on_history", parameters);
}

void UiScript::Hook_OnMainWindowStart()
{
    this->executeFunction("ext_ui_on_main_window_start");
}

void UiScript::Hook_OnScrollbackFrameCreated(Scrollback *window)
{
    QScriptValueList parameters;
    parameters.append(QScriptValue(this->engine, window->GetID()));
    this->executeFunction("ext_ui_on_new_scrollback_frame", parameters);
}
