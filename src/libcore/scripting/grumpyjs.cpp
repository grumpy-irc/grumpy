//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2019

#include "grumpyjs.h"
#include "scriptextension.h"
#include "scriptcommand.h"
#include "../core.h"
#include "../configuration.h"
#include "../commandprocessor.h"
#include "../definitions.h"
#include "../exception.h"
#include "../eventhandler.h"
#include "../generic.h"
#include <QJSValueIterator>

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
    fh.insert("create_timer", "(int interval, string function, [bool start = true]): creates a timer");
    fh.insert("start_timer", "(uint timer, int interval): starts a given timer");
    fh.insert("stop_timer", "(uint timer): stops a given timer");
    fh.insert("destroy_timer", "(uint timer): destroys a timer");
    fh.insert("dump_obj", "(object): returns a string description of object");
    fh.insert("get_startup_time_unix", "(): returns seconds in UNIX time when grumpy started");
    fh.insert("get_uptime", "(): seconds since startup");
    fh.insert("get_startup_date_time", "(): return time object");
    fh.insert("get_current_time_str", "(): return current date and time in string format");
    fh.insert("get_current_time_posix", "(): returns current POSIX time");
    return fh;
}

QString GrumpyJS::get_function_help(const QString& function_name)
{
    return this->script->GetHelpForFunc(function_name);
}

QList<QString> GrumpyJS::get_function_list()
{
    return this->script->GetFunctions();
}

void GrumpyJS::log(const QString& text)
{
    GRUMPY_LOG(text);
}

void GrumpyJS::error_log(const QString& text)
{
    GRUMPY_ERROR(this->script->GetName() + ": " + text);
}

void GrumpyJS::debug_log(const QString& text, int verbosity)
{
    GRUMPY_DEBUG(this->script->GetName() + ": " + text, verbosity);
}

bool GrumpyJS::register_hook(const QString& hook, const QString& function_name)
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

void GrumpyJS::unregister_hook(const QString& hook)
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

bool GrumpyJS::register_cmd(const QString& name, const QString& fc)
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

bool GrumpyJS::has_function(const QString& f)
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

bool GrumpyJS::set_cfg(const QString& key, const QJSValue& value)
{
    Core::GrumpyCore->GetConfiguration()->Extension_SetValue(this->script->GetName(), key, value.toVariant());
    return true;
}

QVariant GrumpyJS::get_cfg(const QString& key)
{
    return Core::GrumpyCore->GetConfiguration()->Extension_GetValue(this->script->GetName(), key);
}

QJSValue GrumpyJS::get_version()
{
    int major = 0;
    int minor = 0;
    int revision = 0;

    Configuration::GetVersion(&major, &minor, &revision);

    // Marshalling
    QJSValue version;
    version.setProperty("Major", major);
    version.setProperty("Minor", minor);
    version.setProperty("Revision", revision);
    version.setProperty("String", QString(GRUMPY_VERSION_STRING));
    return version;
}

QString GrumpyJS::dump_obj(const QJSValue& object, unsigned int indent)
{
    QString indent_prefix = "";
    unsigned int pref = indent;
    while (pref-- > 0)
        indent_prefix += " ";
    QString object_desc = indent_prefix;
    if (object.isArray())
    {
        object_desc += "array [ \n";
        int length = object.property("length").toInt();
        int i = 0;
        while (i < length)
        {
            object_desc += "    " + indent_prefix + dump_obj(object.property(static_cast<quint32>(i++)), indent + 4) + "\n";
        }
        object_desc += indent_prefix + " ]";
    } else if (object.isBool())
    {
        object_desc += "bool (" + Generic::Bool2String(object.toBool()) + ")";
    } else if (object.isCallable())
    {
        object_desc += "callable()";
    } else if (object.isDate())
    {
        object_desc += "datetime (" + object.toDateTime().toString() + ")";
    } else if (object.isError())
    {
        object_desc += "error type";
    } else if (object.isNull())
    {
        object_desc += "null type";
    } else if (object.isNumber())
    {
        object_desc += "int (" + QString::number(object.toInt()) + ")";
    } else if (object.isUndefined())
    {
        object_desc += "undefined type";
    } else if (object.isVariant())
    {
        object_desc += "variant";
    } else if (object.isString())
    {
        object_desc += "string (" + object.toString() + ")";
    } else if (object.isQObject())
    {
        object_desc += "qobject { \n";
        QJSValueIterator it(object);
        while (it.hasNext())
        {
            it.next();
            object_desc += "    " + indent_prefix + it.name() + ": " + dump_obj(it.value(), indent + 4) + "\n";
        }
        object_desc += indent_prefix + " }";
    } else if (object.isObject())
    {
        object_desc += "object { \n";
        QJSValueIterator it(object);
        while (it.hasNext())
        {
            it.next();
            object_desc += "    " + indent_prefix + it.name() + ": " + dump_obj(it.value(), indent + 4) + "\n";
        }
        object_desc += indent_prefix + " }";
    } else
    {
        object_desc += "unknown type";
    }

    return object_desc;
}

QJSValue GrumpyJS::seconds_to_time_span(int seconds)
{
    int days, hours, minutes, secs;
    if (!Generic::SecondsToTimeSpan(seconds, &days, &hours, &minutes, &secs))
        return QJSValue(false);

    QJSValue result = this->script->GetEngine()->newObject();
    result.setProperty("seconds", secs);
    result.setProperty("minutes", minutes);
    result.setProperty("hours", hours);
    result.setProperty("days", days);
    return result;
}

unsigned int GrumpyJS::create_timer(int interval, const QString &function, bool start)
{
    unsigned int timer_id = this->lastTimer++;
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(OnTime()));
    this->timers.insert(timer_id, timer);
    this->timerFunctions.insert(timer, function);
    if (start)
        timer->start(interval);
    return timer_id;
}

bool GrumpyJS::destroy_timer(unsigned int timer)
{
    if (!this->timers.contains(timer))
        return false;
    delete this->timers[timer];
    this->timerFunctions.remove(this->timers[timer]);
    this->timers.remove(timer);
    return true;
}

bool GrumpyJS::start_timer(unsigned int timer, int interval)
{
    if (!this->timers.contains(timer))
        return false;
    this->timers[timer]->start(interval);

    return true;
}

bool GrumpyJS::stop_timer(unsigned int timer)
{
    if (!this->timers.contains(timer))
        return false;

    this->timers[timer]->stop();
    return true;
}

qint64 GrumpyJS::get_startup_time_unix()
{
    // Some older Qt versions don't have toSecsSinceEpoch
    return Core::GrumpyCore->GetConfiguration()->GetStartupDateTime().toMSecsSinceEpoch() / 1000;
}

qint64 GrumpyJS::get_uptime()
{
    return Core::GrumpyCore->GetConfiguration()->GetStartupDateTime().secsTo(QDateTime::currentDateTime());
}

QDateTime GrumpyJS::get_startup_date_time()
{
    return Core::GrumpyCore->GetConfiguration()->GetStartupDateTime();
}

QString GrumpyJS::get_current_time_str()
{
    return QDateTime::currentDateTime().toString();
}

int GrumpyJS::get_current_time_posix()
{
    return static_cast<int>(QDateTime::currentDateTimeUtc().toTime_t());
}

void GrumpyJS::OnTime()
{
    QTimer *timer = dynamic_cast<QTimer*>(QObject::sender());
    if (!this->timerFunctions.contains(timer))
        return;
    this->GetScript()->ExecuteFunction(this->timerFunctions[timer], QJSValueList());
}
