//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef SYSTEMCMDS
#define SYSTEMCMDS

#include "../libcore/definitions.h"
#include "../libcore/commandprocessor.h"
#include <QString>

namespace GrumpyIRC
{
    namespace SystemCmds
    {
        int Exit(SystemCommand *command, CommandArgs command_args);
        int Alias(SystemCommand *command, CommandArgs command_args);
        int UnAlias(SystemCommand *command, CommandArgs command_args);
        int CTCP(SystemCommand *command, CommandArgs command_args);
        int Echo(SystemCommand *command, CommandArgs command_args);
        int Notice(SystemCommand *command, CommandArgs command_args);
        int SendMessage(SystemCommand *command, CommandArgs command_args);
        int NextSessionNick(SystemCommand *command, CommandArgs command_args);
        int Nick(SystemCommand *command, CommandArgs command_args);
        int Netstat(SystemCommand *command, CommandArgs command_args);
        int RAW(SystemCommand *command, CommandArgs command_args);
        int Grumpy(SystemCommand *command, CommandArgs command_args);
        int Act(SystemCommand *command, CommandArgs command_args);
        int GrumpyLink(SystemCommand *command, CommandArgs command_args);
        int Query(SystemCommand *command, CommandArgs command_args);
        int UnsecureGrumpy(SystemCommand *command, CommandArgs command_args);
        int Server(SystemCommand *command, CommandArgs command_args);
        int Topic(SystemCommand *command, CommandArgs command_args);
        int JOIN(SystemCommand *command, CommandArgs command_args);
        int KickBan(SystemCommand *command, CommandArgs command_args);
        int Ban(SystemCommand *command, CommandArgs command_args);
        int KICK(SystemCommand *command, CommandArgs command_args);
        int Uptime(SystemCommand *command, CommandArgs command_args);
        int Script(SystemCommand *command, CommandArgs command_args);
        int RemoveScript(SystemCommand *command, CommandArgs command_args);
        int ScriptList(SystemCommand *command, CommandArgs command_args);
    }
}

#endif // SYSTEMCMDS

