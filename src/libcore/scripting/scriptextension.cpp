//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include "scriptextension.h"
#include "genericjsclass.h"
#include "scriptcommand.h"
#include "scriptexception.h"
#include "grumpyjs_unsafe.h"
#include "scrollbackjs.h"
#include "networkjs.h"
#include "grumpyjs.h"
#include "../core.h"
#include "../configuration.h"
#include "../definitions.h"
#include "../exception.h"
#include "../eventhandler.h"
#include "../networksession.h"
#include "../ircsession.h"
#include "../scrollback.h"
#include "../resources.h"
#include "../../libirc/libircclient/user.h"
#include "../../libirc/libircclient/channel.h"
#include "../../libirc/libircclient/priority.h"
#include "../../libirc/libircclient/network.h"
#include <QFile>

using namespace GrumpyIRC;

QHash<QString, ScriptExtension*>    ScriptExtension::extensions;
QList<QString>                      ScriptExtension::loadedPaths;

ScriptExtension *ScriptExtension::GetExtensionByPath(const QString& path)
{
    foreach (ScriptExtension *extension, ScriptExtension::extensions)
    {
        if (extension->scriptPath == path)
            return extension;
    }

    return nullptr;
}

ScriptExtension *ScriptExtension::GetExtensionByEngine(QJSEngine *e)
{
    foreach (ScriptExtension *extension, ScriptExtension::extensions)
    {
        if (extension->engine == e)
            return extension;
    }

    return nullptr;
}

ScriptExtension *ScriptExtension::GetExtensionByName(const QString& extension_name)
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
        this->DestroyScrollback(this->scriptScbs.first());

    delete this->engine;
    if (this->isLoaded && !this->scriptPath.isEmpty())
        ScriptExtension::loadedPaths.removeAll(this->scriptPath);
    if (this->isLoaded && ScriptExtension::extensions.contains(this->GetName()))
        ScriptExtension::extensions.remove(this->scriptName);
}

bool ScriptExtension::Load(const QString& path, QString *error)
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
        *error = "Unable to read file: " + path;
        return false;
    }
    QTextStream stream(&file);
    QString sx = stream.readAll();
    file.close();
    return this->loadSource(sx, error);
}

bool ScriptExtension::LoadSrc(const QString &unique_id, const QString &source, QString *error)
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

QString ScriptExtension::GetSource()
{
    return this->originalSource;
}

bool ScriptExtension::IsWorking()
{
    if (!this->isWorking || !this->isLoaded)
        return false;

    return this->executeFunctionAsBool("ext_is_working");
}

QJSValue ScriptExtension::ExecuteFunction(const QString &function, const QJSValueList &parameters)
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

bool ScriptExtension::SupportFunction(const QString& name)
{
    return this->functionsExported.contains(name);
}

QString ScriptExtension::GetHelpForFunc(const QString& name)
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
    if (!this->attachedHooks.contains(GRUMPY_SCRIPT_HOOK_SHUTDOWN))
        return;
    this->executeFunction(this->attachedHooks[GRUMPY_SCRIPT_HOOK_SHUTDOWN]);
}

void ScriptExtension::Hook_OnScrollbackDestroyed(Scrollback *scrollback)
{
    if (!this->attachedHooks.contains(GRUMPY_SCRIPT_HOOK_SCROLLBACK_DESTROYED))
        return;
    QJSValueList params;
    params.append(QJSValue(scrollback->GetID()));
    this->executeFunction(this->attachedHooks[GRUMPY_SCRIPT_HOOK_SCROLLBACK_DESTROYED], params);
    if (this->scriptScbs.contains(scrollback))
        this->scriptScbs.removeAll(scrollback);
}

void ScriptExtension::Hook_OnNetworkDisconnect(IRCSession *session)
{
    if (!this->attachedHooks.contains(GRUMPY_SCRIPT_HOOK_NETWORK_DISCONNECT))
        return;
    QJSValueList params;
    params.append(QJSValue(session->GetSID()));
    params.append(QJSValue(session->GetSystemWindow()->GetID()));
    this->executeFunction(this->attachedHooks[GRUMPY_SCRIPT_HOOK_NETWORK_DISCONNECT], params);
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

void ScriptExtension::SubscribeHook(int hook, const QString& function_name)
{
    if (this->attachedHooks.contains(hook))
        this->attachedHooks[hook] = function_name;
    else
        this->attachedHooks.insert(hook, function_name);
}

void ScriptExtension::UnsubscribeHook(int hook)
{
    if (this->attachedHooks.contains(hook))
        this->attachedHooks.remove(hook);
}

bool ScriptExtension::HookSubscribed(int hook)
{
    return this->attachedHooks.contains(hook);
}

int ScriptExtension::GetHookID(const QString &hook)
{
    if (hook == "shutdown")
        return GRUMPY_SCRIPT_HOOK_SHUTDOWN;
    if (hook == "scrollback_destroyed")
        return GRUMPY_SCRIPT_HOOK_SCROLLBACK_DESTROYED;
    if (hook == "scrollback_created")
        return GRUMPY_SCRIPT_HOOK_SCROLLBACK_CREATED;
    if (hook == "network_disconnect")
        return GRUMPY_SCRIPT_HOOK_NETWORK_DISCONNECT;

    return -1;
}

bool ScriptExtension::loadSource(QString source, QString *error)
{
    // Store orig
    this->originalSource = source;
    // Prepend the built-in libs
    source = Resources::GetSource("/grumpy_core/scripting/ecma/libirc.js") + Resources::GetSource("/grumpy_core/scripting/ecma/grumpy.js") + source;
    this->sourceCode = source;
    this->engine = new QJSEngine();

    this->script_ptr = this->engine->evaluate(this->sourceCode);
    if (this->script_ptr.isError())
    {
        *error = "Unable to load script, syntax error at line " + QString::number(this->script_ptr.property("lineNumber").toInt()) + " column " +
                 QString::number(this->script_ptr.property("columnNumber").toInt()) + ": " + this->script_ptr.toString();
        this->isWorking = false;
        return false;
    }
    this->isLoaded = true;
    this->registerFunctions();
    this->registerClasses();
    this->isWorking = true;

    if (!this->IsWorking())
    {
        *error = "Unable to load script " + this->scriptPath + ", ext_is_working() didn't return true";
        this->isWorking = false;
        this->isLoaded = false;
        return false;
    }

    QJSValue info = this->executeFunction("ext_get_info");
    if (info.isUndefined() || info.isNull())
    {
        *error = "Unable to load script " + this->scriptPath + ", ext_get_info() didn't return valid object";
        this->isWorking = false;
        this->isLoaded = false;
        return false;
    }

    if (!info.hasProperty("author") || !info.hasProperty("description") || !info.hasProperty("version") || !info.hasProperty("name"))
    {
        *error = "Unable to load script, ext_get_info() didn't contain some of these properties: name, author, description, version";
        this->isWorking = false;
        this->isLoaded = false;
        return false;
    }

    this->scriptAuthor = info.property("author").toString();
    this->scriptDesc = info.property("description").toString();
    this->scriptName = info.property("name").toString();
    this->scriptVers = info.property("version").toString();

    if (this->scriptName.isEmpty())
    {
        *error = "Unable to load script, invalid extension name (name must be a string)";
        this->isWorking = false;
        this->isLoaded = false;
        return false;
    }

    this->scriptName = this->scriptName.toLower();

    if (ScriptExtension::extensions.contains(this->scriptName))
    {
        *error = this->scriptName + " is already loaded";
        this->isWorking = false;
        this->isLoaded = false;
        return false;
    }

    // Loading is done, let's assume everything works
    ScriptExtension::loadedPaths.append(this->scriptPath);
    ScriptExtension::extensions.insert(this->GetName(), this);
    if (!this->executeFunctionAsBool("ext_init"))
    {
        *error = "Unable to load script, ext_init() didn't return true";
        this->isWorking = false;
        this->isLoaded = false;
        return false;
    }
    return true;
}

bool ScriptExtension::executeFunctionAsBool(const QString &function)
{
    return this->executeFunction(function, QJSValueList()).toBool();
}

QString ScriptExtension::executeFunctionAsString(const QString &function)
{
    return this->executeFunction(function, QJSValueList()).toString();
}

QString ScriptExtension::executeFunctionAsString(const QString &function, const QJSValueList &parameters)
{
    return this->executeFunction(function, parameters).toString();
}

QJSValue ScriptExtension::executeFunction(const QString &function)
{
    return this->executeFunction(function, QJSValueList());
}

QJSValue ScriptExtension::executeFunction(const QString &function, const QJSValueList &parameters)
{
    if (!this->isLoaded)
        throw new ScriptException("Call to script function of extension that isn't loaded", BOOST_CURRENT_FUNCTION, this);

    QJSValue fc = this->engine->globalObject().property(function);
    // If function doesn't exist, undefined value is returned. For most of hooks this is normal, since extensions don't use all of them,
    // so this issue shouldn't be logged anywhere here. Let's just pass the invalid result for callee to handle it themselves.
    if (fc.isUndefined())
        return fc;
    if (!fc.isCallable())
    {
        GRUMPY_ERROR("JS error (" + this->GetName() + "): " + function + " is not a function");
        return fc;
    }
    QJSValue result = fc.call(parameters);
    if (result.isError())
    {
        // There was some error during execution
        qint32 line = result.property("lineNumber").toInt();
        qint32 col = result.property("columnNumber").toInt();
        GRUMPY_ERROR("JS error, line " + QString::number(line) + " column " + QString::number(col) + " (" + this->GetName() + "): " + result.toString());
    }
    return result;
}

void ScriptExtension::registerFunctions()
{
    this->registerHook("ext_init", 0, "(): called on start, must return true, otherwise load of extension is considered as failure");
    this->registerHook("shutdown", 0, "(): called on exit");
    this->registerHook("scrollback_destroyed", 1, "(int scrollback_id): called when scrollback is deleted");
    this->registerHook("network_disconnect", 2, "(network_id, scrollback_id): called when network gets disconnected, provides network ID and system scrollback ID as parameters");
    this->registerHook("ext_get_info", 0, "(): should return version");
    this->registerHook("ext_unload", 0, "(): called when extension is being unloaded from system");
    this->registerHook("ext_is_working", 0, "(): must exist and must return true, if returns false, extension is considered crashed");
}

void ScriptExtension::registerClass(const QString &name, GenericJSClass *c)
{
    QHash<QString, QString> functions = c->GetFunctions();
    foreach (QString function, functions.keys())
    {
        this->functionsExported.append(name + "." + function);
        this->functionsHelp.insert(name + "." + function, functions[function]);
    }
    // Register this class for later removal
    this->classes.append(c);
    this->engine->globalObject().setProperty(name, engine->newQObject(c));
}

void ScriptExtension::registerClasses()
{
    // Check if extension should have access to extra functions
    if (this->IsUnsafe())
    {
        this->registerClass("grumpy_unsafe", new GrumpyJS_Unsafe(this));
    }
    this->registerClass("grumpy", new GrumpyJS(this));
    this->registerClass("grumpy_scrollback", new ScrollbackJS(this));
    this->registerClass("grumpy_network", new NetworkJS(this));
}

void ScriptExtension::registerFunction(const QString &name, const QString &help, bool is_unsafe)
{
    // Check if this script is allowed to access unsafe functions
    if (is_unsafe && !this->isUnsafe)
        return;

    this->functionsExported.append(name);
    this->functionsHelp.insert(name, help);
}

void ScriptExtension::registerHook(const QString &name, int parameters, const QString &help, bool is_unsafe)
{
    Q_UNUSED(parameters);
    this->hooksExported.append(name);
    this->functionsHelp.insert(name, help);
}

bool ScriptExtension::executeFunctionAsBool(const QString &function, const QJSValueList &parameters)
{
    return this->executeFunction(function, parameters).toBool();
}
