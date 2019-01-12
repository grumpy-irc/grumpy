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
#include "scrollback.h"

namespace GrumpyIRC
{
    class Configuration;
    class CommandProcessor;
    class EventHandler;
    class Extension;
    class Factory;
    class Scrollback;

    class LIBCORESHARED_EXPORT Core
    {
        public:
            static Core *GrumpyCore;

            Core();
            virtual ~Core();
            void InitCfg(const QString &home_path = "");
            void LoadCfg();
            CommandProcessor *GetCommandProcessor();
            void SetSystemEventHandler(EventHandler *e);
            EventHandler *GetCurrentEventHandler();
            Configuration *GetConfiguration();
            void InstallFactory(Factory *f);
            Scrollback *NewScrollback(Scrollback *parent, const QString &name, ScrollbackType type);
            void RegisterExtension(Extension *extension);
            void UnregisterExtension(Extension *extension);
            QList<Extension*> GetExtensions();

        private:
            QList<Extension*> extensions;
            Factory *factory;
            bool isLoaded;
            Configuration *config;
            EventHandler *eventHandler;
            CommandProcessor *commandProcessor;
    };
}

#endif // CORE_H
