//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef CORE_H
#define CORE_H

#include "libcore_global.h"

namespace GrumpyIRC
{
    class CommandProcessor;
    class EventHandler;
    class LIBCORESHARED_EXPORT Core
    {
        public:
            static Core *GrumpyCore;

            Core();
            virtual ~Core();
            CommandProcessor *GetCommandProcessor();
            void SetSystemEventHandler(EventHandler *e);
            EventHandler *GetCurrentEventHandler();
        private:
            EventHandler *eventHandler;
            CommandProcessor *commandProcessor;
    };
}

#endif // CORE_H
