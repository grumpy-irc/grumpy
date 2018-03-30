//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "scriptextension.h"
#include "core.h"
#include "exception.h"
#include "eventhandler.h"
#include "networksession.h"
#include "scrollback.h"
#include <QFile>

using namespace GrumpyIRC;

QHash<QString, ScriptExtension*>    ScriptExtension::extensions;
QList<QString>                      ScriptExtension::loadedPaths;

ScriptExtension *ScriptExtension::GetExtensionByPath(QString path)
{
    foreach (ScriptExtension *extension, ScriptExtension::extensions)
    {
        if (extension->scriptPath == path)
            return extension;
    }

    return nullptr;
}

ScriptExtension *ScriptExtension::GetExtensionByEngine(QScriptEngine *e)
{
    foreach (ScriptExtension *extension, ScriptExtension::extensions)
    {
        if (extension->engine == e)
            return extension;
    }

    return nullptr;
}

ScriptExtension *ScriptExtension::GetExtensionByName(QString extension_name)
{
    foreach (ScriptExtension *extension, ScriptExtension::extensions)
    {
        if (extension->scriptName == extension_name)
            return extension;
    }

    return nullptr;
}

QList<ScriptExtension *> ScriptExtension::GetExtensions()
{
    return ScriptExtension::extensions.values();
}

ScriptExtension::ScriptExtension()
{
    this->isWorking = false;
    this->isLoaded = false;
    this->engine = nullptr;
}

ScriptExtension::~ScriptExtension()
{
    // Delete all registered script commands
    while (!this->scriptCmds.isEmpty())
        delete this->scriptCmds.first();

    delete this->engine;
    if (!this->scriptPath.isEmpty())
        ScriptExtension::loadedPaths.removeAll(this->scriptPath);
    if (this->isLoaded && ScriptExtension::extensions.contains(this->GetName()))
        ScriptExtension::extensions.remove(this->scriptName);
}

bool ScriptExtension::Load(QString path, QString *error)
{
    if (this->engine || this->isLoaded)
    {
        *error = "You can't run Load multiple times";
        return false;
    }
    if (ScriptExtension::loadedPaths.contains(path))
    {
        *error = "This script is already loaded";
        return false;
    }

    this->scriptPath = path;

    // Try to read the script
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
    {
        *error = "Unable to read file";
        return false;
    }
    QTextStream stream(&file);
    QString sx = stream.readAll();
    file.close();
    return this->loadSource(sx, error);
}

bool ScriptExtension::LoadSrc(QString unique_id, QString source, QString *error)
{
    if (this->engine || this->isLoaded)
    {
        *error = "You can't run Load multiple times";
        return false;
    }
    if (ScriptExtension::loadedPaths.contains(unique_id))
    {
        *error = "This script is already loaded";
        return false;
    }

    this->scriptPath = unique_id;
    return this->loadSource(source, error);
}

void ScriptExtension::Unload()
{
    if (this->IsWorking())
        this->executeFunction("ext_unload");
    this->isWorking = false;
}

QString ScriptExtension::GetDescription()
{
    return this->scriptDesc;
}

QString ScriptExtension::GetName()
{
    return this->scriptName;
}

QString ScriptExtension::GetVersion()
{
    return this->scriptVers;
}

QString ScriptExtension::GetPath()
{
    return this->scriptPath;
}

QString ScriptExtension::GetAuthor()
{
    return this->scriptAuthor;
}

bool ScriptExtension::IsWorking()
{
    if (!this->isWorking || !this->isLoaded)
        return false;

    return this->executeFunctionAsBool("ext_is_working");
}

QScriptValue ScriptExtension::ExecuteFunction(QString function, QScriptValueList parameters)
{
    this->executeFunction(function, parameters);
}

void ScriptExtension::OnError(QScriptValue e)
{
    GRUMPY_ERROR(this->GetName() + ": exception: " + e.toString());
}

bool ScriptExtension::loadSource(QString source, QString *error)
{
    this->sourceCode = source;
    this->engine = new QScriptEngine();

    connect(this->engine, SIGNAL(signalHandlerException(QScriptValue)), this, SLOT(OnError(QScriptValue)));

    this->script_ptr = this->engine->evaluate(this->sourceCode);
    this->isLoaded = true;
    this->registerFunctions();
    this->isWorking = true;

    if (!this->IsWorking())
    {
        *error = "Unable to load script, ext_is_working() didn't return true";
        this->isWorking = false;
        return false;
    }

    this->scriptAuthor = this->executeFunctionAsString("ext_get_author");
    this->scriptDesc = this->executeFunctionAsString("ext_get_desc");
    this->scriptName = this->executeFunctionAsString("ext_get_name");
    this->scriptVers = this->executeFunctionAsString("ext_get_version");

    if (this->scriptName.isEmpty())
    {
        *error = "Unable to load script, ext_get_name returned nothing";
        this->isWorking = false;
        return false;
    }

    this->scriptName = this->scriptName.toLower();

    if (this->extensions.contains(this->scriptName))
    {
        *error = this->scriptName + " is already loaded";
        this->isWorking = false;
        return false;
    }

    // Loading is done, let's assume everything works
    ScriptExtension::loadedPaths.append(this->scriptPath);
    ScriptExtension::extensions.insert(this->GetName(), this);
    if (!this->executeFunctionAsBool("ext_init"))
    {
        *error = "Unable to load script, ext_init() didn't return true";
        this->isWorking = false;
        return false;
    }
    return true;
}

bool ScriptExtension::executeFunctionAsBool(QString function)
{
    return this->executeFunction(function, QScriptValueList()).toBool();
}

QString ScriptExtension::executeFunctionAsString(QString function)
{
    return this->executeFunction(function, QScriptValueList()).toString();
}

QString ScriptExtension::executeFunctionAsString(QString function, QScriptValueList parameters)
{
    return this->executeFunction(function, parameters).toString();
}

QScriptValue ScriptExtension::executeFunction(QString function)
{
    return this->executeFunction(function, QScriptValueList());
}

QScriptValue ScriptExtension::executeFunction(QString function, QScriptValueList parameters)
{
    if (!this->isLoaded)
        throw new ScriptException("Call to script function of extension that isn't loaded", BOOST_CURRENT_FUNCTION, this);

    QScriptValue fc = this->engine->globalObject().property(function);
    QScriptValue result = fc.call(QScriptValue(), parameters);
    return result;
}

/////////////////////////////////////////
// Function exports
/////////////////////////////////////////

static QScriptValue error_log(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": error_log(text, verbosity): requires 2 parameters");
        return QScriptValue(engine, false);
    }
    QString text = context->argument(0).toString();
    GRUMPY_ERROR(extension->GetName() + ": " + text);
    return QScriptValue(engine, true);
}

static QScriptValue debug_log(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 2)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": debug_log(text, verbosity): requires 2 parameters");
        return QScriptValue(engine, false);
    }
    QString text = context->argument(0).toString();
    int verbosity = context->argument(1).toInt32();
    GRUMPY_DEBUG(extension->GetName() + ": " + text, verbosity);
    return QScriptValue(engine, true);
}

static QScriptValue log(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": log(text): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    QString text = context->argument(0).toString();
    GRUMPY_LOG(text);
    return QScriptValue(engine, true);
}

static QScriptValue network_send_raw(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 2)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": network_send_raw(window_id, text): requires 2 parameters");
        return QScriptValue(engine, false);
    }
    unsigned int window_id = context->argument(0).toUInt32();
    QString text = context->argument(1).toString();
    Scrollback *w = Scrollback::GetScrollbackByID(window_id);
    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": network_send_raw(window_id, text): unknown scrollback");
        return QScriptValue(engine, false);
    }
    if (!w->GetSession())
    {
        GRUMPY_ERROR(extension->GetName() + ": network_send_raw(window_id, text): scrollback is not connected to IRC network");
        return QScriptValue(engine, false);
    }

    w->GetSession()->SendRaw(w, text);

    return QScriptValue(engine, true);
}

static QScriptValue scrollback_write(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 2)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": scrollback_write(window_id, text): requires 2 parameters");
        return QScriptValue(engine, false);
    }
    unsigned int window_id = context->argument(0).toUInt32();
    QString text = context->argument(1).toString();
    Scrollback *w = Scrollback::GetScrollbackByID(window_id);
    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": scrollback_write(window_id, text): unknown scrollback");
        return QScriptValue(engine, false);
    }
    w->InsertText(text);

    return QScriptValue(engine, true);
}

static QScriptValue scrollback_get_target(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": scrollback_get_target(window_id): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    unsigned int window_id = context->argument(0).toUInt32();
    Scrollback *w = Scrollback::GetScrollbackByID(window_id);
    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": scrollback_get_target(window_id): unknown scrollback");
        return QScriptValue(engine, false);
    }

    return QScriptValue(engine, w->GetTarget());
}

static QScriptValue register_cmd(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 2)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": register_cmd(command_name, callback): requires 2 parameters");
        return QScriptValue(engine, false);
    }
    QString cmd_name = context->argument(0).toString().toLower();
    if (cmd_name.contains(" "))
    {
        GRUMPY_ERROR(extension->GetName() + ": register_cmd(command_name, callback): invalid command name: " + cmd_name);
        return QScriptValue(engine, false);
    }
    QString fc = context->argument(1).toString();
    if (Core::GrumpyCore->GetCommandProcessor()->Exists(cmd_name))
    {
        GRUMPY_ERROR(extension->GetName() + ": register_cmd(command_name, callback): command already registered: " + cmd_name);
        return QScriptValue(engine, false);
    }
    ScriptCommand *command = new ScriptCommand(cmd_name, extension, fc);
    Core::GrumpyCore->GetCommandProcessor()->RegisterCommand(command);
    return QScriptValue(engine, true);
}

void ScriptExtension::registerFunctions()
{
    this->engine->globalObject().setProperty("grumpy_register_cmd", this->engine->newFunction(register_cmd, 2));
    this->engine->globalObject().setProperty("grumpy_debug_log", this->engine->newFunction(debug_log, 2));
    this->engine->globalObject().setProperty("grumpy_error_log", this->engine->newFunction(error_log, 1));
    this->engine->globalObject().setProperty("grumpy_log", this->engine->newFunction(log, 1));
    this->engine->globalObject().setProperty("grumpy_scrollback_write", this->engine->newFunction(scrollback_write, 2));
    this->engine->globalObject().setProperty("grumpy_scrollback_get_target", this->engine->newFunction(scrollback_get_target, 1));
    this->engine->globalObject().setProperty("grumpy_network_send_raw", this->engine->newFunction(network_send_raw, 2));
}

bool ScriptExtension::executeFunctionAsBool(QString function, QScriptValueList parameters)
{
    return this->executeFunction(function, parameters).toBool();
}

ScriptException::ScriptException(QString Message, QString function_id, ScriptExtension *extension) : Exception(Message, function_id)
{

}

ScriptException::~ScriptException()
{

}

ScriptCommand::ScriptCommand(QString name, ScriptExtension *e, QString function) : SystemCommand(name, nullptr)
{
    this->fn = function;
    this->script = e;
    e->scriptCmds.append(this);
}

ScriptCommand::~ScriptCommand()
{
    this->script->scriptCmds.removeAll(this);
    Core::GrumpyCore->GetCommandProcessor()->UnregisterCommand(this);
}

ScriptExtension *ScriptCommand::GetScript()
{
    return this->script;
}

QString ScriptCommand::GetFN()
{
    return this->fn;
}

QScriptEngine *ScriptCommand::GetEngine()
{
    return this->script->engine;
}

int ScriptCommand::Run(CommandArgs args)
{
    QScriptValueList parameters;
    parameters.append(QScriptValue(this->GetEngine(), args.Window->GetID()));
    parameters.append(QScriptValue(this->GetEngine(), args.ParameterLine));
    this->script->executeFunction(this->fn, parameters);
    return 0;
}