//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef LIBEVENTHANDLER_H
#define LIBEVENTHANDLER_H

#include "libcore_global.h"
#include "../libirc/libircclient/irceventhandler.h"

// Debug
#define GRUMPY_DEBUG(text, verbosity)    if (GrumpyIRC::Core::GrumpyCore && GrumpyIRC::Core::GrumpyCore->GetCurrentEventHandler()) \
                                         { GrumpyIRC::Core::GrumpyCore->GetCurrentEventHandler()->OnDebug(text, verbosity); }
// Standard log
#define GRUMPY_LOG(text)                 if (GrumpyIRC::Core::GrumpyCore && GrumpyIRC::Core::GrumpyCore->GetCurrentEventHandler()) \
                                         { GrumpyIRC::Core::GrumpyCore->GetCurrentEventHandler()->OnSystemLog(text); }
// Error message
#define GRUMPY_ERROR(text)    if (GrumpyIRC::Core::GrumpyCore && GrumpyIRC::Core::GrumpyCore->GetCurrentEventHandler()) \
                                         { GrumpyIRC::Core::GrumpyCore->GetCurrentEventHandler()->OnError(text); }

namespace GrumpyIRC
{
    class LIBCORESHARED_EXPORT EventHandler : libircclient::IRCEventHandler
    {
        public:
            EventHandler();
            ~EventHandler();
            virtual void OnMessage(unsigned long long ScrollbackID)=0;
            virtual void OnDebug(QString text, unsigned int verbosity = 1)=0;
            virtual void OnError(QString text)=0;
            virtual void OnSystemLog(QString text)=0;

    };
}

#endif // EVENTHANDLER_H
