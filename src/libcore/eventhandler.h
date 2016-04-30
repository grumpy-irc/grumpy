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

#include "definitions.h"


#include "libcore_global.h"
#include "../libirc/libircclient/irceventhandler.h"

// Debug
#define GRUMPY_DEBUG(text, verbosity)    { if (GrumpyIRC::Core::GrumpyCore && GrumpyIRC::Core::GrumpyCore->GetCurrentEventHandler()) \
                                         { GrumpyIRC::Core::GrumpyCore->GetCurrentEventHandler()->OnDebug(text, verbosity); } }
// Standard log
#define GRUMPY_LOG(text)                 { if (GrumpyIRC::Core::GrumpyCore && GrumpyIRC::Core::GrumpyCore->GetCurrentEventHandler()) \
                                         { GrumpyIRC::Core::GrumpyCore->GetCurrentEventHandler()->OnSystemLog(text); } }
// Error message
#define GRUMPY_ERROR(text)               { if (GrumpyIRC::Core::GrumpyCore && GrumpyIRC::Core::GrumpyCore->GetCurrentEventHandler()) \
                                         { GrumpyIRC::Core::GrumpyCore->GetCurrentEventHandler()->OnError(text); } }

namespace GrumpyIRC
{
    class GrumpydSession;

    class LIBCORESHARED_EXPORT EventHandler : libircclient::IRCEventHandler
    {
        public:
            EventHandler();
            virtual ~EventHandler();
            virtual void OnMessage(scrollback_id_t ScrollbackID);
            virtual void OnDebug(QString text, unsigned int verbosity = 1);
            virtual void OnError(QString text);
            virtual void OnSystemLog(QString text);
            virtual void OnGrumpydCtorCall(GrumpydSession *session);
            virtual void OnGrumpydDtorCall(GrumpydSession *session);
    };
}

#endif // EVENTHANDLER_H
