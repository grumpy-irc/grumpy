//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef GRUMPYEVENTHANDLER_H
#define GRUMPYEVENTHANDLER_H

#include "grumpy_global.h"
#include <libcore/eventhandler.h>

namespace GrumpyIRC
{
    class LIBGRUMPYSHARED_EXPORT GrumpyEventHandler : public EventHandler
	{
		public:
            GrumpyEventHandler() = default;
            void OnMessage(scrollback_id_t ScrollbackID) override;
            void OnDebug(const QString &text, unsigned int verbosity = 1) override;
            void OnError(const QString &text) override;
            void OnSystemLog(const QString &text) override;

	};
}

#endif // GRUMPYEVENTHANDLER_H
