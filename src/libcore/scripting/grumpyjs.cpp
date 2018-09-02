//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "grumpyjs.h"
#include "scriptextension.h"
#include "scriptcommand.h"
#include "../core.h"
#include "../configuration.h"
#include "../commandprocessor.h"
#include "../definitions.h"
#include "../exception.h"
#include "../eventhandler.h"

using namespace GrumpyIRC;

GrumpyJS::GrumpyJS(ScriptExtension *s) : GenericJSClass(s)
{

}

GrumpyJS::~GrumpyJS()
{
    qDeleteAll(this->scriptCmds);
    this->scriptCmds.clear();
}

QHash<QString, QString> GrumpyJS::GetFunctions()
{
    QHash<QString, QString> fh;
    fh.insert("get_function_help", "(string function): return help for a function");
    fh.insert("get_function_list", "(): return list of functions");
    fh.insert("log", "(string text): writes text to system log");
    fh.insert("debug_log", "(string text): writes to debug log");
    fh.insert("error_log", "(string text): write to error log");
    fh.insert("is_unsafe", "(): returns if unsafe functions are enabled");
    fh.insert("register_cmd", "(command, function): register new grumpy command that will execute function");
    fh.insert("get_hook_list", "(): returns a list of all hooks");
    fh.insert("get_version", "(): returns version object with properties: Major, Minor, Revision, String");
    fh.insert("set_cfg", "(key, value): stores value as key in settings");
    fh.insert("get_cfg", "(key, default): returns stored value from ini file");
    fh.insert("has_function", "(function_name): return true or false whether function is present");
    fh.insert("get_context", "(): return execution context, either core, grumpyd or GrumpyChat (core doesn't have ui functions and hooks)");
    fh.insert("get_context_id", "(): return execution context id, either core, grumpyd or GrumpyChat");
    fh.insert("register_hook", "(string hook, string function_id): creates a hook");
    fh.insert("unregister_hook", "(string hook): removes hook");
    return fh;
}

QString GrumpyJS::get_function_help(QString function_name)
{
    return this->script->GetHelpForFunc(function_name);
}

QList<QString> GrumpyJS::get_function_list()
{
    return this->script->GetFunctions();
}

void GrumpyJS::log(QString text)
{
    GRUMPY_LOG(text);
}

void GrumpyJS::error_log(QString text)
{
    GRUMPY_ERROR(this->script->GetName() + ": " + text);
}

void GrumpyJS::debug_log(QString text, int verbosity)
{
    GRUMPY_DEBUG(this->script->GetName() + ": " + text, verbosity);
}

bool GrumpyJS::register_hook(QString hook, QString function_name)
{
    int hook_id = this->script->GetHookID(hook);
    if (hook_id < 0)
    {
        GRUMPY_ERROR(this->script->GetName() + ": register_hook(hook, fc): unknown hook: " + hook);
        return false;
    }
    this->script->SubscribeHook(hook_id, function_name);
    return true;
}

void GrumpyJS::unregister_hook(QString hook)
{
    int hook_id = this->script->GetHookID(hook);
    if (hook_id < 0)
    {
        GRUMPY_ERROR(this->script->GetName() + ": unregister_hook(h): unknown hook: " + hook);
        return;
    }
    this->script->UnsubscribeHook(hook_id);
}

bool GrumpyJS::is_unsafe()
{
    return this->script->IsUnsafe();
}

bool GrumpyJS::register_cmd(QString name, QString fc)
{
    if (name.contains(" ") || name.size() > 128)
    {
        GRUMPY_ERROR(this->script->GetName() + ": register_cmd(command_name, callback): invalid command name: " + name);
        return false;
    }
    if (Core::GrumpyCore->GetCommandProcessor()->Exists(name))
    {
        GRUMPY_ERROR(this->script->GetName() + ": register_cmd(command_name, callback): command already registered: " + name);
        return false;
    }
    ScriptCommand *command = new ScriptCommand(name, this->script, fc);
    this->scriptCmds.append(command);
    Core::GrumpyCore->GetCommandProcessor()->RegisterCommand(command);
    return true;
}

QList<QString> GrumpyJS::get_hook_list()
{
    return this->script->GetHooks();
}

bool GrumpyJS::has_function(QString f)
{
    return this->script->SupportFunction(f);
}

QString GrumpyJS::get_context()
{
    return this->script->GetContext();
}

int GrumpyJS::get_context_id()
{
    return this->script->GetContextID();
}

bool GrumpyJS::set_cfg(QString key, QJSValue value)
{
    Core::GrumpyCore->GetConfiguration()->Extension_SetValue(this->script->GetName(), key, value.toVariant());
    return true;
}

QVariant GrumpyJS::get_cfg(QString key)
{
    return Core::GrumpyCore->GetConfiguration()->Extension_GetValue(this->script->GetName(), key);
}

QJSValue GrumpyJS::get_version()
{
    int major = 0;
    int minor = 0;
    int revision = 0;

    Core::GrumpyCore->GetConfiguration()->GetVersion(&major, &minor, &revision);

    // Marshalling
    QJSValue version;
    version.setProperty("Major", major);
    version.setProperty("Minor", minor);
    version.setProperty("Revision", revision);
    version.setProperty("String", QString(GRUMPY_VERSION_STRING));
    return version;
}
