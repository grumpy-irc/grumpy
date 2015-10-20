//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "grumpyd.h"
#include "corewrapper.h"
#include "databasexml.h"
#include "sleeper.h"
#include "listener.h"
#include "../libcore/core.h"
#include "../libcore/eventhandler.h"
#include "../libcore/gp.h"

using namespace GrumpyIRC;

Grumpyd::Grumpyd()
{
    running = true;
    this->listener = new Listener();
}

Grumpyd::~Grumpyd()
{
    delete this->listener;
}

void Grumpyd::Main()
{
    GRUMPY_LOG("Loading database");
    this->databaseBackend = new DatabaseXML();
    this->databaseBackend->LoadUsers();
    GRUMPY_LOG("Starting listeners");
    this->listener->listen(QHostAddress::Any, GP_DEFAULT_PORT);
    GRUMPY_LOG("Listener open on port " + QString::number(GP_DEFAULT_PORT));
}

