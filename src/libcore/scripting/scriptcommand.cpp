//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2019

#include "scriptcommand.h"
#include "scriptextension.h"
#include "../core.h"

using namespace GrumpyIRC;

ScriptCommand::ScriptCommand(const QString &name, ScriptExtension *e, const QString &function) : SystemCommand(name, nullptr)
{
    this->fn = function;
    this->script = e;
}

ScriptCommand::~ScriptCommand()
{
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

QJSEngine *ScriptCommand::GetEngine()
{
    return this->script->engine;
}

int ScriptCommand::Run(const CommandArgs &args)
{
    QJSValueList parameters;
    parameters.append(QJSValue(args.SrcScrollback->GetID()));
    parameters.append(QJSValue(args.ParameterLine));
    this->script->executeFunction(this->fn, parameters);
    return 0;
}
