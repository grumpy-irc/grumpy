//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "commandprocessor.h"
#include "core.h"
#include "scrollback.h"
#include "ircsession.h"
#include "../libirc/libircclient/network.h"
#include "eventhandler.h"

using namespace GrumpyIRC;

CommandProcessor::CommandProcessor()
{
    this->CommandPrefix = '/';
}

CommandProcessor::~CommandProcessor()
{
    foreach (QString command, this->CommandList.keys())
        delete this->CommandList[command];
}

void CommandProcessor::RegisterCommand(SystemCommand *sc)
{
	GRUMPY_DEBUG("Registering system command: " + sc->GetName(), 2);
    QString name = sc->GetName().toLower();
    this->CommandList.insert(name, sc);
}

int CommandProcessor::ProcessText(QString text, Scrollback *window)
{
    text.replace("\r", "");
    QStringList items = text.split("\n");
    foreach (QString line, items)
    {
        int return_code = this->ProcessItem(line, window);
        switch (-return_code)
        {
            case COMMANDPROCESSOR_ENOTEXIST:
                GRUMPY_ERROR("Unknown command");
                break;
        }
    }
    return 0;
}

int CommandProcessor::ProcessItem(QString command, Scrollback *window)
{
    command = command.trimmed();
    if (command.isEmpty())
        return 0;
    // Check if command starts with prefix, but make sure it doesn't start with double prefix, which means ignore the prefix symbol
    if (command.startsWith(this->CommandPrefix) && !(command.length() > 2 && command[1] == this->CommandPrefix))
    {
        // This is a system command
		command = command.mid(1);
        CommandArgs parameters;
        QString command_name = command.toLower();
        // Get parameters
        if (command.contains(" "))
        {
            QString param = command.mid(command.indexOf(" ") + 1);
            parameters.ParameterLine = param;
            parameters.Parameters = param.split(" ");
            command_name = command.mid(0, command.indexOf(" ")).toLower();
        }
        if (!this->CommandList.contains(command_name))
        {
            if (window && window->GetSession())
            {
                // We are connected to some IRC network, we will transfer this as IRC command
                command_name = command_name.toUpper();
                if (parameters.ParameterLine.isEmpty())
                    window->GetSession()->GetNetwork()->TransferRaw(command_name);
                else
                    window->GetSession()->GetNetwork()->TransferRaw(command_name + " " + parameters.ParameterLine);
                return 0;
            }
            return -COMMANDPROCESSOR_ENOTEXIST;
        }
        return this->CommandList[command_name]->Run(parameters);
    }
    // It's not a command, let's do something with this, probably send it to active channel as message?
    return 0;
}

SystemCommand::SystemCommand(QString name, SC_Callback callback)
{
    this->_name = name;
    this->Callback = callback;
}

SystemCommand::~SystemCommand()
{

}

QString SystemCommand::GetName()
{
    return this->_name;
}

int SystemCommand::Run(CommandArgs args)
{
    return this->Callback(this, args);
}
