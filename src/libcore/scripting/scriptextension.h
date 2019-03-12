//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef SCRIPTEXTENSION_H
#define SCRIPTEXTENSION_H

#include "../extension.h"
#include <QJSEngine>
#include <QHash>

#define GRUMPY_SCRIPT_HOOK_SHUTDOWN                     0
#define GRUMPY_SCRIPT_HOOK_SCROLLBACK_DESTROYED         1

namespace GrumpyIRC
{
    class ScriptCommand;
    class GenericJSClass;
    class LIBCORESHARED_EXPORT ScriptExtension : public Extension
    {
            Q_OBJECT
        public:
            static ScriptExtension *GetExtensionByPath(QString path);
            static ScriptExtension *GetExtensionByEngine(QJSEngine *e);
            static ScriptExtension *GetExtensionByName(QString extension_name);
            static QList<ScriptExtension*> GetExtensions();

            ScriptExtension();
            ~ScriptExtension();
            bool Load(QString path, QString *error);
            bool LoadSrc(QString unique_id, QString source, QString *error);
            virtual void Unload();
            QString GetDescription();
            QString GetName();
            QString GetVersion();
            QString GetPath();
            QString GetAuthor();
            QString GetSource();
            bool IsWorking();
            QJSValue ExecuteFunction(QString function, QJSValueList parameters);
            virtual unsigned int GetContextID();
            virtual QString GetContext();
            bool IsUnsafe();
            bool SupportFunction(QString name);
            QString GetHelpForFunc(QString name);
            QList<QString> GetHooks();
            QList<QString> GetFunctions();
            void Hook_Shutdown();
            void Hook_OnScrollbackDestroyed(Scrollback *scrollback);
            virtual void RegisterScrollback(Scrollback *sc);
            virtual void DestroyScrollback(Scrollback *sc);
            virtual bool HasScrollback(Scrollback *sc);
            void SubscribeHook(int hook, QString function_name);
            void UnsubscribeHook(int hook);
            bool HookSubscribed(int hook);
            virtual int GetHookID(QString hook);

        protected:
            static QList<QString> loadedPaths;
            static QHash<QString, ScriptExtension*> extensions;
            bool loadSource(QString source, QString *error);
            bool executeFunctionAsBool(QString function, QJSValueList parameters);
            bool executeFunctionAsBool(QString function);
            QString executeFunctionAsString(QString function);
            QString executeFunctionAsString(QString function, QJSValueList parameters);
            QJSValue executeFunction(QString function, QJSValueList parameters);
            QJSValue executeFunction(QString function);
            virtual void registerFunction(QString name, QString help = "", bool is_unsafe = false);
            virtual void registerClass(QString name, GenericJSClass *c);
            virtual void registerClasses();
            virtual void registerHook(QString name, int parameters, QString help = "", bool is_unsafe = false);
            //! Makes all functions available to ECMA
            virtual void registerFunctions();
            QJSEngine *engine;
            QJSValue script_ptr;
            QList<QString> hooksExported;
            QList<QString> functionsExported;
            QHash<QString, QString> functionsHelp;
            QString originalSource;
            QString sourceCode;
            QString scriptPath;
            QString scriptName;
            QString scriptDesc;
            QString scriptAuthor;
            QString scriptVers;
            bool isWorking;
            bool isLoaded;
            bool isUnsafe;
            QList<QString> externalCallbacks;
            QList<GenericJSClass*> classes;
            QHash<int, QString> attachedHooks;
            QList<Scrollback*> scriptScbs;
            friend class ScriptCommand;
    };
}

#endif // SCRIPTEXTENSION_H
