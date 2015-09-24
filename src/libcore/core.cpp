//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "core.h"
#include "commandprocessor.h"
#include "eventhandler.h"

GrumpyIRC::Core *GrumpyIRC::Core::GrumpyCore = NULL;

GrumpyIRC::Core::Core()
{
    this->commandProcessor = new CommandProcessor();
    this->eventHandler = NULL;
    GrumpyCore = this;
}

GrumpyIRC::Core::~Core()
{
    delete this->commandProcessor;
    delete this->eventHandler;
}

GrumpyIRC::CommandProcessor *GrumpyIRC::Core::GetCommandProcessor()
{
    return this->commandProcessor;
}

void GrumpyIRC::Core::SetSystemEventHandler(GrumpyIRC::EventHandler *e)
{
    this->eventHandler = e;
}

GrumpyIRC::EventHandler *GrumpyIRC::Core::GetCurrentEventHandler()
{
    return this->eventHandler;
}
