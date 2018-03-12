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
#include "exception.h"
#include "scrollback.h"
#include "grumpydsession.h"
#include "ircsession.h"
#include "generic.h"
#include "../libirc/libircclient/network.h"
#include "eventhandler.h"

using namespace GrumpyIRC;

CommandProcessor::CommandProcessor()
{
    this->SplitLong = true;
    this->LongSize = 300;
    this->MinimalSize = 20;
    this->CommentChar = '#';
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

int CommandProcessor::ProcessText(QString text, Scrollback *window, bool comments_rm)
{
    text.replace("\r", "");
    QStringList items = text.split("\n");
    foreach (QString line, items)
    {
        if (comments_rm)
        {
            if (text.trimmed().startsWith(this->CommentChar))
                continue;
        }
        int return_code = this->ProcessItem(line, window);
        switch (-return_code)
        {
            case COMMANDPROCESSOR_ENOTEXIST:
                GRUMPY_ERROR("Unknown command");
                break;
            case COMMANDPROCESSOR_ENOTCONNECTED:
                GRUMPY_ERROR("Not connected");
                break;
            case COMMANDPROCESSOR_ETOOMANYREDS:
                GRUMPY_ERROR("Too many redirects in hash table");
                break;
        }
    }
    return 0;
}

static QList<QString> Split(int size, int minimal_size, QString text)
{
    QList<QString> messages;
    while (text.size() > size)
    {
        // We need to find a part of text which is divided by space
        if (!text.mid(0, size).contains(" "))
        {
            QString trimmed_part = text.mid(size);
            if (trimmed_part.size() < minimal_size)
            {
                // The remaining part of text is too short, we don't want to send message this short because it looks weird
                // let's make it bigger by shortening the previous part and making this one longer
                int proper_size = size - minimal_size;
                trimmed_part = text.mid(proper_size);
                messages.append(text.mid(0, proper_size));
            } else
            {
                messages.append(text.mid(0, size));
            }
            text = trimmed_part;
        } else
        {
            QString shortened_text = text.mid(0, size);
            shortened_text = shortened_text.mid(0, shortened_text.lastIndexOf(" "));
            QString remaining_text = text.mid(shortened_text.length() + 1);
            if (remaining_text.size() < minimal_size)
            {
                // Too short, make big
                shortened_text = text.mid(0, size - minimal_size);
                shortened_text = shortened_text.mid(0, shortened_text.lastIndexOf(" "));
                remaining_text = text.mid(shortened_text.length() + 1);
            }
            text = remaining_text;
            messages.append(shortened_text);
        }
    }
    if (!text.isEmpty())
        messages.append(text);

    return messages;
}

QList<QString> CommandProcessor::GetCommands()
{
    return this->CommandList.keys();
}

QHash<QString, QString> CommandProcessor::GetAliasRdTable()
{
    return this->aliasList;
}

QList<QString> GrumpyIRC::CommandProcessor::GetAList()
{
    return this->aliasList.keys();
}

bool GrumpyIRC::CommandProcessor::Exists(QString name) const
{
    return this->CommandList.contains(name) || this->aliasList.contains(name);
}

void GrumpyIRC::CommandProcessor::RegisterAlias(QString name, QString target)
{
    if (this->aliasList.contains(name))
        return;
    this->aliasList.insert(name, target);
}

int CommandProcessor::ProcessItem(QString command, Scrollback *window)
{
    command = command.trimmed();
    if (command.isEmpty())
        return 0;
    // Check if command starts with prefix, but make sure it doesn't start with double prefix, which means ignore the prefix symbol
    if (command.startsWith(this->CommandPrefix) && !(command.length() > 1 && command[1] == this->CommandPrefix))
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
        int recursion_cnt = 0;
        QString temp1;
        while (!this->CommandList.contains(command_name) && this->aliasList.contains(command_name))
        {
            if (recursion_cnt++ > 20)
                return -COMMANDPROCESSOR_ETOOMANYREDS;
            command_name = this->aliasList[command_name];
            if (command_name.contains(" "))
            {
                // This is an alias which contains also parameter, we store it as a temporary string which is
                // used in case this is a final cmd name
                // otherwise we continue searching others
                if (temp1.isEmpty())
                    temp1 = command_name.mid(command_name.indexOf(" ") + 1);
                else
                    temp1 = command_name.mid(command_name.indexOf(" ") + 1) + " " + temp1;
                command_name = command_name.mid(0, command_name.indexOf(" "));
            }
        }
        if (!temp1.isEmpty())
        {
            // we need to prepend this parameters to current parameters, there should be space in some cases to separate them
            QString space = "";
            if (!parameters.ParameterLine.isEmpty())
                space = " ";
            parameters.ParameterLine = temp1 + space + parameters.ParameterLine;
            // now insert the extra parameters
            QStringList temp2 = temp1.split(" ");
            temp2.append(parameters.Parameters);
            parameters.Parameters = temp2;
        }
        if (!this->CommandList.contains(command_name))
        {
            if (window && window->GetSession())
            {
                if (!window->GetSession()->IsConnected())
                    return -COMMANDPROCESSOR_ENOTCONNECTED;
                // We are connected to some IRC network, we will transfer this as IRC command
                command_name = command_name.toUpper();
                if (parameters.ParameterLine.isEmpty())
                    window->GetSession()->SendRaw(window, command_name);
                else
                    window->GetSession()->SendRaw(window, command_name + " " + parameters.ParameterLine);
                return 0;
            }
            return -COMMANDPROCESSOR_ENOTEXIST;
        }
        return this->CommandList[command_name]->Run(parameters);
    }
    if (command.startsWith(this->CommandPrefix) && (command.length() > 1 && command[1] == this->CommandPrefix))
        command = command.mid(1);
    // It's not a command, let's do something with this
    if (window->IsDead() != true && (window->GetType() == ScrollbackType_Channel || window->GetType() == ScrollbackType_User))
    {
        // This is a channel window, so we send this as a message to the channel
        if (this->SplitLong && command.size() > static_cast<int>(this->LongSize))
        {
            QList<QString> messages = Split(static_cast<int>(this->LongSize), static_cast<int>(this->MinimalSize), command);
            foreach (QString text, messages)
                window->GetSession()->SendMessage(window, text);
            return 0;
        }
        window->GetSession()->SendMessage(window, command);
    }
    else
    {
        window->InsertText("You can't send messages to this window", ScrollbackItemType_SystemWarning);
    }
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
