//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef UISCRIPT_H
#define UISCRIPT_H

#include "grumpy_global.h"
#include <libcore/scriptextension.h>

namespace GrumpyIRC
{
    /*!
     * \brief The UiScript class overloads ScriptExtension so that GUI of GrumpyChat is available to ECMA scripts
     */
    class LIBGRUMPYSHARED_EXPORT UiScript : public ScriptExtension
    {
        public:
            UiScript();
            QString GetContext();
            unsigned int GetContextID();
            void Hook_OnExit();
            void Hook_OnHistory(Scrollback *window, QString text);
            void Hook_WindowSwitch(Scrollback *window);
            void Hook_OnMainWindowStart();
            void Hook_OnScrollbackFrameCreated(Scrollback *window);
        protected:
            void registerFunctions();
    };
}

#endif // UISCRIPT_H
