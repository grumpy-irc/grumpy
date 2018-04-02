//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "grumpyscript.h"
#include "core.h"
#include "configuration.h"
#include <QCoreApplication>

using namespace GrumpyIRC;

QString GrumpyScript::ReplaceVars(QString source, QHash<QString, QString> vars, char prefix, char escape)
{
    int current_pos = 0;
    QList<QChar> separators;
    separators << ' ';
    QString replaced = source;
    while (current_pos < replaced.size())
    {
        char symbol = replaced[current_pos].toLatin1();
        char previous = '\0';
        if (current_pos > 0)
            previous = replaced[current_pos - 1].toLatin1();
        if (symbol == prefix && previous != escape)
        {
            // This looks like a variable
            QString variable;
            int variable_start = current_pos;
            int variable_end = current_pos;
            // find its end
            while (variable_end < replaced.size() && !separators.contains(replaced[variable_end]))
                variable_end++;
            variable = replaced.mid(variable_start, variable_end - variable_start);
            if (!variable.isEmpty() && vars.contains(variable.mid(1)))
            {
                // this is a known 
            }
        }
        current_pos++;
    }
    return replaced;
}

QString GrumpyScript::ReplaceStdVars(QString text)
{
    text.replace("$grumpy.version", Core::GrumpyCore->GetConfiguration()->GetVersion());
    text.replace("$grumpy.paths.home", Core::GrumpyCore->GetConfiguration()->GetHomePath());
    text.replace("$grumpy.paths.binary", QCoreApplication::applicationDirPath());
    text.replace("$grumpy.paths.home.script", Core::GrumpyCore->GetConfiguration()->GetHomeScriptPath());
    text.replace("$grumpy.paths.home.extension", Core::GrumpyCore->GetConfiguration()->GetHomeExtensionPath());
    text.replace("$grumpy.paths.extension", Core::GrumpyCore->GetConfiguration()->GetExtensionPath());
    text.replace("$grumpy.paths.script", Core::GrumpyCore->GetConfiguration()->GetScriptPath());
    return text;
}

GrumpyScript::GrumpyScript()
{

}

void GrumpyScript::Exec()
{

}

