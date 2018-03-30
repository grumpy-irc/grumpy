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
#include "grumpyeventhandler.h"
#include "scrollbackframe.h"
#include "inputbox.h"
#include "scrollbacksmanager.h"
#include "../libcore/core.h"
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

void UiScript::Hook_WindowSwitch(Scrollback *window)
{
    QScriptValueList parameters;
    parameters.append(QScriptValue(this->engine, window->GetID()));
    this->executeFunction("ext_ui_on_window_switch", parameters);
}

static QScriptValue load_history(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    if (context->argumentCount() < 2)
    {
        GRUMPY_ERROR(extension->GetName() + ": load_history(window, text): requires 2 parameters");
        return QScriptValue(engine, false);
    }
    ScrollbackFrame *win = ScrollbacksManager::Global->GetWindowFromID(context->argument(0).toUInt32());
    if (!win)
    {
        GRUMPY_ERROR(extension->GetName() + ": load_history(window, text): unknown frame");
        return QScriptValue(engine, false);
    }
    QList<QString> input = context->argument(1).toString().split("\n");
    win->GetInputBox()->LoadHistory(input);
    return QScriptValue(engine, true);
}

static QScriptValue clean_history(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    if (context->argumentCount() < 1)
    {
        GRUMPY_ERROR(extension->GetName() + ": clean_history(window, text): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    ScrollbackFrame *win = ScrollbacksManager::Global->GetWindowFromID(context->argument(0).toUInt32());
    if (!win)
    {
        GRUMPY_ERROR(extension->GetName() + ": clean_history(window, text): unknown frame");
        return QScriptValue(engine, false);
    }
    QList<QString> input = context->argument(1).toString().split("\n");
    win->GetInputBox()->ClearHistory();
    return QScriptValue(engine, true);
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

void UiScript::registerFunctions()
{
    this->engine->globalObject().setProperty("grumpy_ui_load_history", this->engine->newFunction(load_history, 2));
    this->engine->globalObject().setProperty("grumpy_ui_wipe_history", this->engine->newFunction(clean_history, 2));
    ScriptExtension::registerFunctions();
}
