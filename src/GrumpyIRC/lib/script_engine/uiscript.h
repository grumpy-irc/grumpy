//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2019

#ifndef UISCRIPT_H
#define UISCRIPT_H

#include "../grumpy_global.h"
#include <libcore/scripting/scriptextension.h>

#define GRUMPY_SCRIPT_HOOK_UI_SCROLLBACK_CREATED    100000
#define GRUMPY_SCRIPT_HOOK_UI_MAIN_LOAD             100001
#define GRUMPY_SCRIPT_HOOK_UI_EXIT                  100002
#define GRUMPY_SCRIPT_HOOK_UI_HISTORY               100003
#define GRUMPY_SCRIPT_HOOK_UI_WINDOW_SWITCH         100004

namespace GrumpyIRC
{
    /*!
     * \brief The UiScript class overloads ScriptExtension so that GUI of GrumpyChat is available to ECMA scripts
     */
    class LIBGRUMPYSHARED_EXPORT UiScript : public ScriptExtension
    {
        public:
            UiScript();
            QString GetContext() override;
            unsigned int GetContextID() override;
            void Hook_OnExit();
            void Hook_OnHistory(Scrollback *window, const QString& text);
            void Hook_WindowSwitch(Scrollback *window);
            void Hook_OnMainWindowStart();
            void Hook_OnScrollbackFrameCreated(Scrollback *window);
            int GetHookID(const QString &hook) override;

        protected:
            void registerFunctions() override;
            void registerClasses() override;
    };
}

#endif // UISCRIPT_H
