//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include "configuration.h"
#include "core.h"
#include "commandprocessor.h"
#include "extension.h"
#include "factory.h"
#include "eventhandler.h"
#include <QDir>

GrumpyIRC::Core *GrumpyIRC::Core::GrumpyCore = nullptr;

GrumpyIRC::Core::Core()
{
    this->commandProcessor = new CommandProcessor();
    this->config = nullptr;
    this->eventHandler = new EventHandler();
    GrumpyCore = this;
    this->factory = new Factory();
}

GrumpyIRC::Core::~Core()
{
    // Unload all extensions
    foreach (Extension *extension, this->extensions)
    {
        extension->Hook_Shutdown();
        delete extension;
    }
    this->extensions.clear();
    delete this->config;
    delete this->factory;
    delete this->commandProcessor;
    delete this->eventHandler;
    GrumpyCore = nullptr;
}

GrumpyIRC::Scrollback *GrumpyIRC::Core::NewScrollback(GrumpyIRC::Scrollback *parent, const QString &name, ScrollbackType type)
{
    return this->factory->NewScrollback(parent, name, type);
}

void GrumpyIRC::Core::RegisterExtension(GrumpyIRC::Extension *extension)
{
    if (!this->extensions.contains(extension))
        this->extensions.append(extension);
}

void GrumpyIRC::Core::UnregisterExtension(GrumpyIRC::Extension *extension)
{
    if (this->extensions.contains(extension))
        this->extensions.removeAll(extension);
}

QList<GrumpyIRC::Extension *> GrumpyIRC::Core::GetExtensions()
{
    return this->extensions;
}

void GrumpyIRC::Core::InstallFactory(Factory *f)
{
    delete this->factory;
    this->factory = f;
}

void GrumpyIRC::Core::InitCfg(const QString &home_path)
{
    this->config = new Configuration();

    if (!home_path.isEmpty())
    {
        GRUMPY_DEBUG("Home: " + home_path, 1);
        if (!QDir().exists(home_path))
            QDir().mkpath(home_path);
        this->config->SetHomePath(home_path);
        this->config->SetAlternativeConfigFile(home_path + QDir::separator() + CONFIGURATION_FILE);
    }
}

void GrumpyIRC::Core::LoadCfg()
{
    this->config->Load();
}

GrumpyIRC::CommandProcessor *GrumpyIRC::Core::GetCommandProcessor()
{
    return this->commandProcessor;
}

void GrumpyIRC::Core::SetSystemEventHandler(GrumpyIRC::EventHandler *e)
{
    delete this->eventHandler;
    this->eventHandler = e;
}

GrumpyIRC::EventHandler *GrumpyIRC::Core::GetCurrentEventHandler()
{
    return this->eventHandler;
}

GrumpyIRC::Configuration *GrumpyIRC::Core::GetConfiguration()
{
    return this->config;
}
