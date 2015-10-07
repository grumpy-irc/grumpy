//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef DEFAULTCONFIG
#define DEFAULTCONFIG

//#include "../libcore/configuration.h"

#ifdef GCFG
    #undef GCFG
#endif

#define GCFG CoreWrapper::GrumpyCore->GetConfiguration()
// Nickname
#define CONFIG_NICK GCFG->GetValueAsString("nick", "GrumpyUser")
#define SET_CONFIG_NICK(nick) GCFG->SetValue("nick", QVariant(nick))

// Default line format
#define CONFIG_LINE_FORMAT GCFG->GetValueAsString("line_format", "($time) $string")
#define SET_CONFIG_LINE_FORMAT(format) GCFG->SetValue("line_format", QVariant(format))

// Default message format
#define CONFIG_MESSAGE_FORMAT GCFG->GetValueAsString("message_format", "<$nick> $string")
#define SET_CONFIG_MESSAGE_FORMAT(format) GCFG->SetValue("message_format", QVariant(format))

#define CONFIG_ACTION_FORMAT GCFG->GetValueAsString("action_format", "* $nick $string")
#define SET_CONFIG_ACTION_FORMAT(format) GCFG->SetValue("action_format", QVariant(format))

#endif // DEFAULTCONFIG

