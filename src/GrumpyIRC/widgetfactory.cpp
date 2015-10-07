//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "../libcore/exception.h"
#include "widgetfactory.h"
#include "scrollbacksmanager.h"
#include "mainwindow.h"

using namespace GrumpyIRC;

#define MANAGER MainWindow::Main->GetScrollbackManager()

WidgetFactory::WidgetFactory()
{

}

// This function is called by core when a new scrollback is created
// because here we have a gui, we need to create a wrapper for it
Scrollback *WidgetFactory::NewScrollback(Scrollback *parent, QString name, ScrollbackType type)
{
    Scrollback *sx = Factory::NewScrollback(parent, name, type);
    if (MainWindow::Main == NULL)
        throw new GrumpyIRC::NullPointerException("MainWindow *MainWindow::Main", BOOST_CURRENT_FUNCTION);
    // Create a new scrollback wrapper for this one
    MANAGER->CreateWindow(name, MANAGER->GetWindowFromScrollback(parent), false, true, sx);
    return sx;
}

