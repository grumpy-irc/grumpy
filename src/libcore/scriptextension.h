//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef SCRIPTEXTENSION_H
#define SCRIPTEXTENSION_H

#include "extension.h"
#include "exception.h"
#include <QtScript>
#include <QHash>

namespace GrumpyIRC
{
    class ScriptExtension : public Extension
    {
        public:
            static ScriptExtension *GetExtensionByPath(QString path);
            static ScriptExtension *GetExtensionByEngine(QScriptEngine *e);
            static ScriptExtension *GetExtensionByName(QString extension_name);
            static QList<ScriptExtension*> GetExtensions();

            ScriptExtension();
            ~ScriptExtension();
            bool Load(QString path, QString *error);
            bool LoadSrc(QString unique_id, QString source, QString *error);
            void Unload();
            QString GetDescription();
            QString GetName();
            QString GetVersion();
            QString GetPath();
            QString GetAuthor();
            bool IsWorking();

        private:
            static QList<QString> loadedPaths;
            static QHash<QString, ScriptExtension*> extensions;
            bool loadSource(QString source, QString *error);
            bool executeFunctionAsBool(QString function, QScriptValueList parameters);
            bool executeFunctionAsBool(QString function);
            QString executeFunctionAsString(QString function);
            QString executeFunctionAsString(QString function, QScriptValueList parameters);
            QScriptValue executeFunction(QString function, QScriptValueList parameters);
            QScriptValue executeFunction(QString function);
            //! Makes all functions available to ECMA
            void registerFunctions();
            QScriptEngine *engine;
            QScriptValue script_ptr;
            QString sourceCode;
            QString scriptPath;
            QString scriptName;
            QString scriptDesc;
            QString scriptAuthor;
            QString scriptVers;
            bool isWorking;
            bool isLoaded;
    };

    class ScriptException : public Exception
    {
        public:
            ScriptException(QString Message, QString function_id, ScriptExtension *extension);
            ~ScriptException();
    };
}

#endif // SCRIPTEXTENSION_H
