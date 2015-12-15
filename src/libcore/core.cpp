//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "configuration.h"
#include "core.h"
#include "commandprocessor.h"
#include "factory.h"
#include "eventhandler.h"

GrumpyIRC::Core *GrumpyIRC::Core::GrumpyCore = NULL;

GrumpyIRC::Core::Core()
{
    this->commandProcessor = new CommandProcessor();
    this->config = NULL;
    this->eventHandler = new EventHandler();
    GrumpyCore = this;
    this->factory = new Factory();
}

GrumpyIRC::Core::~Core()
{
    delete this->config;
    delete this->factory;
    delete this->commandProcessor;
    delete this->eventHandler;
}

GrumpyIRC::Scrollback *GrumpyIRC::Core::NewScrollback(GrumpyIRC::Scrollback *parent, QString name, ScrollbackType type)
{
    return this->factory->NewScrollback(parent, name, type);
}

void GrumpyIRC::Core::InstallFactory(Factory *f)
{
    delete this->factory;
    this->factory = f;
}

void GrumpyIRC::Core::InitCfg()
{
    this->config = new Configuration();
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
