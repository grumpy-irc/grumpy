//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef GDEVENTHANDLER_H
#define GDEVENTHANDLER_H

#include "../libcore/definitions.h"
#include "../libcore/eventhandler.h"

namespace GrumpyIRC
{
    class GDEventHandler : public EventHandler
    {
        public:
            GDEventHandler()=default;
#ifdef __linux__
            void OnDebug(const QString &text, unsigned int verbosity = 1) override;
            void OnError(const QString &text) override;
            void OnSystemLog(const QString &text) override;
#endif
    };
}

#endif // GDEVENTHANDLER_H
