//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "commandprocessor.h"
#include "core.h"
#include "exception.h"
#include "scrollback.h"
#include "grumpydsession.h"
#include "ircsession.h"
#include "generic.h"
#include "../libirc/libircclient/network.h"
#include "../libirc/libircclient/user.h"
#include "eventhandler.h"

using namespace GrumpyIRC;

CommandProcessor::CommandProcessor()
{
    this->SplitLong = true;
    this->LongSize = 512;
    this->AutoReduceMsgSize = true;
    this->MinimalSize = 20;
    this->CommentChar = '#';
    this->CommandPrefix = '/';
}

CommandProcessor::~CommandProcessor()
{
    foreach (QString command, this->commandList.keys())
        delete this->commandList[command];
}

void CommandProcessor::RegisterCommand(SystemCommand *sc)
{
    QString lower_name = sc->GetName().toLower();

    // In case this command is already registered throw exception
    if (this->commandList.contains(lower_name))
        throw new Exception("Command already exists: " + lower_name, BOOST_CURRENT_FUNCTION);

    GRUMPY_DEBUG("Registering system command: " + sc->GetName(), 2);
    this->commandList.insert(lower_name, sc);
}

void CommandProcessor::UnregisterCommand(SystemCommand *sc)
{
    QString lower_name = sc->GetName().toLower();

    if (!this->commandList.contains(lower_name))
        return;

    GRUMPY_DEBUG("Unregistering system command: " + sc->GetName(), 2);
    this->commandList.remove(lower_name);
}

int CommandProcessor::ProcessText(QString text, Scrollback *scrollback, bool comments_rm)
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
        int return_code = this->ProcessItem(line, scrollback);
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
    // Minimal size must be at least 2 times smaller than size
    if (size < minimal_size * 2)
        size = minimal_size * 2;
    while (text.size() > size)
    {
        // We need to find a part of text which is divided by space
        if (!text.midRef(0, size).contains(" "))
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
    return this->commandList.keys();
}

QHash<QString, QString> CommandProcessor::GetAliasRdTable()
{
    return this->aliasList;
}

QList<QString> GrumpyIRC::CommandProcessor::GetAList()
{
    return this->aliasList.keys();
}

bool GrumpyIRC::CommandProcessor::Exists(const QString& name) const
{
    return this->commandList.contains(name) || this->aliasList.contains(name);
}

void GrumpyIRC::CommandProcessor::RegisterAlias(const QString& name, const QString& target)
{
    if (this->aliasList.contains(name))
        return;
    this->aliasList.insert(name, target);
}

void CommandProcessor::UnregisterAlias(const QString& name)
{
    if (this->aliasList.contains(name))
        this->aliasList.remove(name);
}

int CommandProcessor::ProcessItem(QString command, Scrollback *scrollback)
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
        parameters.SrcScrollback = scrollback;
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
        while (!this->commandList.contains(command_name) && this->aliasList.contains(command_name))
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
        if (!this->commandList.contains(command_name))
        {
            if (scrollback && scrollback->GetSession())
            {
                if (!scrollback->GetSession()->IsConnected())
                    return -COMMANDPROCESSOR_ENOTCONNECTED;
                // We are connected to some IRC network, we will transfer this as IRC command
                command_name = command_name.toUpper();
                if (parameters.ParameterLine.isEmpty())
                    scrollback->GetSession()->SendRaw(scrollback, command_name);
                else
                    scrollback->GetSession()->SendRaw(scrollback, command_name + " " + parameters.ParameterLine);
                return 0;
            }
            return -COMMANDPROCESSOR_ENOTEXIST;
        }
        return this->commandList[command_name]->Run(parameters);
    }
    if (command.startsWith(this->CommandPrefix) && (command.length() > 1 && command[1] == this->CommandPrefix))
        command = command.mid(1);
    // It's not a command, let's do something with this
    if (scrollback->IsDead() != true && (scrollback->GetType() == ScrollbackType_Channel || scrollback->GetType() == ScrollbackType_User))
    {
        // This is a channel window, so we send this as a message to the channel
        // If the message is too long we split it
        int long_size = this->LongSize;
        int minm_size = this->MinimalSize;
        if (this->SplitLong && this->AutoReduceMsgSize)
        {
            // THIS IS NOT SENDING ANYTHING ANYWHERE - IT JUST TRIES TO PREDICT THE RESULTING LENGTH OF MESSAGE AS RELAYED
            // BY IRCD TO CLIENTS!! It's an ugly hack, but it does the job. And it's optional :)
            // We automatically reduce the maximal size of the message by length of user string and channel so that we comply with RFC
            // part of each message from server to clients:
            // :user!ident@hostname PRIVMSG #channel_name :text of message
            // ^           this text                      ^  <- needs to be considered part of message (the length of this)
            libircclient::User *self = scrollback->GetSession()->GetSelfNetworkID(scrollback);
            // In extreme case this could be NULL
            if (self)
            {
                // Let's not really contcatenate this for perfomance reason and to prevent people bitching when reviewing the code :)
                // QString server_str      = ":" + self->ToString()          + " PRIVMSG " + scrollback->GetTarget() + " :";
                int server_side_str_length = 1   + self->ToString().length() + 9           + scrollback->GetTarget().length() + 3; // (2 for " :" + 1 for \0)
                long_size -= server_side_str_length;
            }
        }
        // failsafe for extreme cases and invalid config
        if (long_size < 20)
            long_size = 20;
        if ((minm_size * 2) > long_size)
            minm_size = long_size / 2;
        if (this->SplitLong && command.size() > long_size)
        {
            QList<QString> messages = Split(long_size, minm_size, command);
            foreach (QString text, messages)
                scrollback->GetSession()->SendMessage(scrollback, text);
            return 0;
        }
        scrollback->GetSession()->SendMessage(scrollback, command);
    }
    else
    {
        scrollback->InsertText("You can't send messages to this window", ScrollbackItemType_SystemWarning);
    }
    return 0;
}

SystemCommand::SystemCommand(const QString &name, SC_Callback callback)
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

int SystemCommand::Run(const CommandArgs &args)
{
    return this->Callback(this, args);
}
