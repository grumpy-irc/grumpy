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
#include "configuration.h"
#include "definitions.h"
#include "exception.h"
#include "eventhandler.h"
#include "networksession.h"
#include "scrollback.h"
#include "resources.h"
#include "../libirc/libircclient/user.h"
#include "../libirc/libircclient/channel.h"
#include "../libirc/libircclient/priority.h"
#include "../libirc/libircclient/network.h"
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
    this->isUnsafe = Core::GrumpyCore->GetConfiguration()->GetUnsafeScriptFc();
}

ScriptExtension::~ScriptExtension()
{
    // Delete all windows that belong to this extension
    while (!this->scriptScbs.isEmpty())
        delete this->scriptScbs.first();

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
    return this->executeFunction(function, parameters);
}

unsigned int ScriptExtension::GetContextID()
{
    return GRUMPY_SCRIPT_CONTEXT_CORE;
}

QString ScriptExtension::GetContext()
{
    return "core";
}

bool ScriptExtension::IsUnsafe()
{
    return this->isUnsafe;
}

bool ScriptExtension::SupportFunction(QString name)
{
    return this->functionsExported.contains(name);
}

QString ScriptExtension::GetHelpForFunc(QString name)
{
    if (!this->functionsHelp.contains(name))
        return QString();
    return this->functionsHelp[name];
}

QList<QString> ScriptExtension::GetHooks()
{
    return this->hooksExported;
}

QList<QString> ScriptExtension::GetFunctions()
{
    return this->functionsExported;
}

void ScriptExtension::Hook_Shutdown()
{
    this->executeFunction("ext_on_shutdown");
}

void ScriptExtension::Hook_OnScrollbackDestroyed(Scrollback *scrollback)
{
    QScriptValueList params;
    params.append(QScriptValue(this->engine, scrollback->GetID()));
    this->executeFunction("ext_on_scrollback_destroyed", params);
    if (this->scriptScbs.contains(scrollback))
        this->scriptScbs.removeAll(scrollback);
}

void ScriptExtension::RegisterScrollback(Scrollback *sc)
{
    this->scriptScbs.append(sc);
}

void ScriptExtension::DestroyScrollback(Scrollback *sc)
{
    if (!this->scriptScbs.contains(sc))
        return;
    this->scriptScbs.removeAll(sc);
    sc->Close();
}

bool ScriptExtension::HasScrollback(Scrollback *sc)
{
    return this->scriptScbs.contains(sc);
}

void ScriptExtension::OnError(QScriptValue e)
{
    GRUMPY_ERROR(this->GetName() + ": exception: " + e.toString());
}

bool ScriptExtension::loadSource(QString source, QString *error)
{
    QScriptEngine syntax_check;
    QScriptSyntaxCheckResult s = syntax_check.checkSyntax(source);
    if (s.state() != QScriptSyntaxCheckResult::Valid)
    {
        *error = "Unable to load script, syntax error at line " + QString::number(s.errorLineNumber()) + " column " + QString::number(s.errorColumnNumber()) + ": " + s.errorMessage();
        this->isWorking = false;
        return false;
    }

    // Prepend the built-in libs
    source = Resources::GetSource("/grumpy_core/ecma/libirc.js") + Resources::GetSource("/grumpy_core/ecma/grumpy.js") + source;

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
    // If function doesn't exist, invalid value is returned. For most of hooks this is normal, since extensions don't use all of them,
    // so this issue shouldn't be logged anywhere here. Let's just pass the invalid result for callee to handle it themselves.
    if (!fc.isValid())
        return fc;
    if (!fc.isFunction())
    {
        GRUMPY_ERROR("JS error (" + this->GetName() + "): " + function + " is not a function");
        return fc;
    }
    QScriptValue result = fc.call(QScriptValue(), parameters);
    if (result.isError())
    {
        // There was some error during execution
        qint32 line = result.property("lineNumber").toInt32();
        GRUMPY_ERROR("JS error, line " + QString::number(line) + " (" + this->GetName() + "): " + result.toString());
    }
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

static NetworkSession *meta_network_session_get(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return nullptr;
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": network_get_meta(window_id): requires 1 parameter1");
        return nullptr;
    }
    unsigned int window_id = context->argument(0).toUInt32();
    Scrollback *w = Scrollback::GetScrollbackByID(window_id);
    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": network_get_meta(window_id): unknown scrollback");
        return nullptr;
    }
    if (!w->GetSession())
    {
        GRUMPY_ERROR(extension->GetName() + ": network_get_meta(window_id): scrollback is not connected to IRC network");
        return nullptr;
    }

    return w->GetSession();
}

static libircclient::Network *meta_network_get(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return nullptr;
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": meta_network_get(window_id): requires 1 parameter1");
        return nullptr;
    }
    unsigned int window_id = context->argument(0).toUInt32();
    Scrollback *w = Scrollback::GetScrollbackByID(window_id);
    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": meta_network_get(window_id): unknown scrollback");
        return nullptr;
    }
    NetworkSession *session = w->GetSession();
    if (!session)
    {
        GRUMPY_ERROR(extension->GetName() + ": meta_network_get(window_id): scrollback is not connected to IRC network");
        return nullptr;
    }
    libircclient::Network *network = session->GetNetwork(w);
    if (!network)
    {
        GRUMPY_ERROR(extension->GetName() + ": meta_network_get(window_id): scrollback has NULL IRC network");
        return nullptr;
    }

    return network;
}

static QScriptValue network_get_nick(QScriptContext *context, QScriptEngine *engine)
{
    libircclient::Network *network = meta_network_get(context, engine);
    if (!network)
        return QScriptValue(engine, false);

    return QScriptValue(engine, network->GetNick());
}

static QScriptValue network_get_ident(QScriptContext *context, QScriptEngine *engine)
{
    libircclient::Network *network = meta_network_get(context, engine);
    if (!network)
        return QScriptValue(engine, false);

    return QScriptValue(engine, network->GetIdent());
}

static QScriptValue network_get_host(QScriptContext *context, QScriptEngine *engine)
{
    libircclient::Network *network = meta_network_get(context, engine);
    if (!network)
        return QScriptValue(engine, false);

    return QScriptValue(engine, network->GetHost());
}

static QScriptValue network_get_network_name(QScriptContext *context, QScriptEngine *engine)
{
    libircclient::Network *network = meta_network_get(context, engine);
    if (!network)
        return QScriptValue(engine, false);

    return QScriptValue(engine, network->GetNetworkName());
}

static QScriptValue network_get_server_host(QScriptContext *context, QScriptEngine *engine)
{
    libircclient::Network *network = meta_network_get(context, engine);
    if (!network)
        return QScriptValue(engine, false);

    return QScriptValue(engine, network->GetServerAddress());
}

static QScriptValue network_send_message(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 3)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": network_send_message(window_id, target, text): requires 3 parameters");
        return QScriptValue(engine, false);
    }
    unsigned int window_id = context->argument(0).toUInt32();
    QString target = context->argument(1).toString();
    QString text = context->argument(2).toString();
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
    w->GetSession()->SendMessage(w, target, text);
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

static QScriptValue scrollback_has_network(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": scrollback_has_network(window_id): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    unsigned int window_id = context->argument(0).toUInt32();
    Scrollback *w = Scrollback::GetScrollbackByID(window_id);
    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": scrollback_has_network(window_id): unknown scrollback");
        return QScriptValue(engine, false);
    }
    if (!w->GetSession())
        return QScriptValue(engine, false);
    if (!w->GetSession()->GetNetwork(w))
        return QScriptValue(engine, false);

    return QScriptValue(engine, true);
}

static QScriptValue scrollback_has_network_session(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": scrollback_has_network_session(window_id): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    unsigned int window_id = context->argument(0).toUInt32();
    Scrollback *w = Scrollback::GetScrollbackByID(window_id);
    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": scrollback_has_network_session(window_id): unknown scrollback");
        return QScriptValue(engine, false);
    }
    if (!w->GetSession())
    {
        return QScriptValue(engine, false);
    }

    return QScriptValue(engine, true);
}

static QScriptValue scrollback_get_type(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": scrollback_get_type(window_id): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    unsigned int window_id = context->argument(0).toUInt32();
    Scrollback *w = Scrollback::GetScrollbackByID(window_id);
    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": scrollback_get_type(window_id): unknown scrollback");
        return QScriptValue(engine, false);
    }
    QString type = "unknown";
    switch (w->GetType())
    {
        case ScrollbackType_Channel:
            return "channel";
        case ScrollbackType_System:
            return "system";
        case ScrollbackType_User:
            return "user";
    }

    return QScriptValue(engine, type);
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

static QScriptValue scrollback_new(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 2)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": scrollback_new(parent, name): requires 2 parameters");
        return QScriptValue(engine, false);
    }
    unsigned int parent_id = context->argument(0).toUInt32();
    Scrollback *parent;
    if (!parent_id)
        parent = nullptr;
    else
        parent = Scrollback::GetScrollbackByID(parent_id);
    if (parent_id && !parent)
    {
        GRUMPY_ERROR(extension->GetName() + ": scrollback_new(parent, name): parent scrollback not found");
        return QScriptValue(engine, false);
    }
    QString name = context->argument(1).toString();
    Scrollback *w = Core::GrumpyCore->NewScrollback(parent, name, ScrollbackType_System);

    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": scrollback_new(parent, name): unknown error");
        return QScriptValue(engine, false);
    }

    extension->RegisterScrollback(w);

    return QScriptValue(engine, w->GetID());
}

static QScriptValue scrollback_delete(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": scrollback_delete(parent, name): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    unsigned int id = context->argument(0).toUInt32();
    Scrollback *w = Scrollback::GetScrollbackByID(id);
    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": scrollback_delete(parent, name): scrollback not found");
        return QScriptValue(engine, false);
    }

    if (!extension->HasScrollback(w))
    {
        GRUMPY_ERROR(extension->GetName() + ": scrollback_delete(parent, name): scrollback doesn't belong to extension");
        return QScriptValue(engine, false);
    }

    extension->DestroyScrollback(w);
    return QScriptValue(engine, true);
}

static QScriptValue get_scrollback_list(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    QList<scrollback_id_t> scrollback_list;
    Scrollback::ScrollbackList_Mutex.lock();
    foreach (Scrollback *s, Scrollback::ScrollbackList)
    {
        scrollback_list.append(s->GetID());
    }
    Scrollback::ScrollbackList_Mutex.unlock();

    return qScriptValueFromSequence(engine, scrollback_list);
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

static QScriptValue get_cfg(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": get_cfg(key): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    QScriptValue default_value;
    if (context->argumentCount() > 1)
        default_value = context->argument(1);
    QString key = context->argument(0).toString();

    if (!Core::GrumpyCore->GetConfiguration()->Extension_Contains(extension->GetName(), key))
        return default_value;

    QVariant value = Core::GrumpyCore->GetConfiguration()->Extension_GetValue(extension->GetName(), key);
    QScriptValue result;
    switch (value.type())
    {
        case QVariant::Bool:
            result = QScriptValue(engine, value.toBool());
            break;
        case QVariant::Int:
            result = QScriptValue(engine, value.toInt());
            break;
        case QVariant::String:
            result = QScriptValue(engine, value.toString());
            break;
        default:
            result = default_value;
            break;
    }

    return result;
}

static QScriptValue set_cfg(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    if (context->argumentCount() < 2)
    {
        GRUMPY_ERROR(extension->GetName() + ": set_cfg(key, value): requires 2 parameters");
        return QScriptValue(engine, false);
    }
    QString key = context->argument(0).toString();
    QVariant value = context->argument(1).toVariant();

    Core::GrumpyCore->GetConfiguration()->Extension_SetValue(extension->GetName(), key, value);
    return QScriptValue();
}

static QScriptValue get_version(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    int major = 0;
    int minor = 0;
    int revision = 0;

    Core::GrumpyCore->GetConfiguration()->GetVersion(&major, &minor, &revision);

    // Marshalling
    QScriptValue version = engine->newObject();
    version.setProperty("Major", QScriptValue(engine, major));
    version.setProperty("Minor", QScriptValue(engine, minor));
    version.setProperty("Revision", QScriptValue(engine, revision));
    version.setProperty("String", QString(GRUMPY_VERSION_STRING));

    return version;
}

static QScriptValue get_context(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    return QScriptValue(engine, extension->GetContext());
}

static QScriptValue has_function(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": has_function(function): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    QString fx = context->argument(0).toString();
    return QScriptValue(engine, extension->SupportFunction(fx));
}

static QScriptValue get_function_help(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": get_function_help(function): requires 1 parameter");
        return QScriptValue(engine, QString("Function not found"));
    }
    QString fx = context->argument(0).toString();
    return QScriptValue(engine, extension->GetHelpForFunc(fx));
}

static QScriptValue get_function_list(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    return qScriptValueFromSequence(engine, extension->GetFunctions());
}

static QScriptValue process(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 2)
    {
        // Wrong number of parameters
        GRUMPY_ERROR(extension->GetName() + ": process(window_id, input): requires 2 parameters");
        return QScriptValue(engine, false);
    }
    unsigned int window_id = context->argument(0).toUInt32();
    QString x = context->argument(1).toString();
    Scrollback *w = Scrollback::GetScrollbackByID(window_id);
    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": process(window_id): unknown scrollback");
        return QScriptValue(engine, false);
    }

    return QScriptValue(engine, Core::GrumpyCore->GetCommandProcessor()->ProcessText(x, w));
}

static QScriptValue get_hook_list(QScriptContext *context, QScriptEngine *engine)
{
    ScriptExtension *extension = ScriptExtension::GetExtensionByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    return qScriptValueFromSequence(engine, extension->GetHooks());
}

void ScriptExtension::registerFunctions()
{
    this->registerFunction("grumpy_get_function_help", get_function_help, 1, "(function_name): give you help for a function, returns string");
    this->registerFunction("grumpy_get_function_list", get_function_list, 0, "(): returns array with list of functions");
    this->registerFunction("grumpy_get_hook_list", get_hook_list, 0, "(): returns a list of all hooks");
    this->registerFunction("grumpy_get_version", get_version, 0, "(): returns version object with properties: Major, Minor, Revision, String");
    this->registerFunction("grumpy_set_cfg", set_cfg, 2, "(key, value): stores value as key in settings");
    this->registerFunction("grumpy_get_cfg", get_cfg, 2, "(key, default): returns stored value from ini file");
    this->registerFunction("grumpy_has_function", has_function, 1, "(function_name): return true or false whether function is present");
    this->registerFunction("grumpy_get_context", get_context, 0, "(): return execution context, either core or GrumpyChat (core doesn't have ui functions and hooks)");
    this->registerFunction("grumpy_network_send_raw", network_send_raw, 2, "(scrollback_id, text): sends a RAW data to network that belongs to scrollback");
    this->registerFunction("grumpy_register_cmd", register_cmd, 2, "(command, function): register new grumpy command that will execute function");
    this->registerFunction("grumpy_debug_log", debug_log, 2, "(text, verbosity): prints to debug log");
    this->registerFunction("grumpy_error_log", error_log, 1, "(text): prints to log");
    this->registerFunction("grumpy_log", log, 1, "(text): prints to log");
    this->registerFunction("grumpy_get_scrollback_list", get_scrollback_list, 0, "(): returns a list of all scrollback IDs");
    this->registerFunction("grumpy_scrollback_new", scrollback_new, 2, "(parent, name): creates a new scrollback, parent can be 0 if this should be root scrollback, "\
                                                                       "returns false on error otherwise scrollback_id");
    this->registerFunction("grumpy_scrollback_delete", scrollback_delete, 1, "(scrollback_id): destroy scrollback, can be only used for scrollbacks created with this script");
    this->registerFunction("grumpy_scrollback_write", scrollback_write, 2, "(scrollback_id, text): write text");
    this->registerFunction("grumpy_scrollback_get_type", scrollback_get_type, 1, "(scrollback_id): return type of scrollback; system, channel, user");
    this->registerFunction("grumpy_scrollback_get_target", scrollback_get_target, 1, "(scrollback_id): return target name of scrollback (channel name, user name)");
    this->registerFunction("grumpy_scrollback_has_network", scrollback_has_network, 1, "(scrollback_id): return true if scrollback belongs to network");
    this->registerFunction("grumpy_scrollback_has_network_session", scrollback_has_network_session, 1, "(scrollback_id): returns true if scrollback has existing IRC session");
    this->registerFunction("grumpy_network_get_nick", network_get_nick, 1, "(scrollback_id): return your nickname for network of scrollback");
    this->registerFunction("grumpy_network_get_ident", network_get_ident, 1, "(scrollback_id): return your ident for network of scrollback");
    this->registerFunction("grumpy_network_get_host", network_get_host, 1, "(scrollback_id): return your host for network of scrollback");
    this->registerFunction("grumpy_network_get_server_host", network_get_server_host, 1, "(scrollback_id): return server hostname for network of scrollback");
    this->registerFunction("grumpy_network_get_network_name", network_get_network_name, 1, "(scrollback_id): return network for network of scrollback");
    this->registerFunction("grumpy_network_send_message", network_send_message, 3, "(scrollback_id, target, message): sends a silent message to target for network of scrollback");
    this->registerFunction("grumpy_network_send_raw", network_send_raw, 2, "(scrollback_id, text): sends RAW data to network of scrollback");
    this->registerFunction("grumpy_process", process, 2, "(window_id, text): !unsafe! sends input to command processor, esentially same as entering text to input box in program", true);

    this->registerHook("ext_on_shutdown", 0, "(): called on exit");
    this->registerHook("ext_on_scrollback_destroyed", 1, "(int scrollback_id): called when scrollback is deleted");
    this->registerHook("ext_get_name", 0, "(): should return a name of this extension");
    this->registerHook("ext_get_desc", 0, "(): should return description");
    this->registerHook("ext_get_author", 0, "(): should contain name of creator");
    this->registerHook("ext_desc_version", 0, "(): should return version");
}

void ScriptExtension::registerFunction(QString name, QScriptEngine::FunctionSignature function_signature, int parameters, QString help, bool is_unsafe)
{
    // Check if this script is allowed to access unsafe functions
    if (is_unsafe && !this->isUnsafe)
        return;

    this->functionsExported.append(name);
    this->functionsHelp.insert(name, help);
    this->engine->globalObject().setProperty(name, this->engine->newFunction(function_signature, parameters));
}

void ScriptExtension::registerHook(QString name, int parameters, QString help, bool is_unsafe)
{
    this->hooksExported.append(name);
    this->functionsHelp.insert(name, help);
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
    parameters.append(QScriptValue(this->GetEngine(), args.SrcScrollback->GetID()));
    parameters.append(QScriptValue(this->GetEngine(), args.ParameterLine));
    this->script->executeFunction(this->fn, parameters);
    return 0;
}
