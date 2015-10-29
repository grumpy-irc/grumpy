//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

// This file must be included first because it defines GRUMPY_WIN
#include "../libcore/definitions.h"

#ifdef GRUMPY_WIN
    #include <windows.h>
#endif
#include "mainwindow.h"
#include "corewrapper.h"
#include "grumpyeventhandler.h"
#include "../libcore/autocompletionengine.h"
#include "inputbox.h"
#include "widgetfactory.h"
#include "../libcore/core.h"
#include <QApplication>

using namespace GrumpyIRC;

#ifdef GRUMPY_WIN
// Normally we would compile this program so it has no console window, but because we are hackers, we want to see a boot log from its startup
// this function hides the console once the grumpy is started up and main window is loaded

// it doesn't do anything if grumpy is compiled in debug mode, because we want to see it all time in that case :)
void HideConsole()
{
#ifndef _DEBUG
    HWND Stealth;
    AllocConsole();
    Stealth = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(Stealth, 0);
#endif
}
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialize core first
    CoreWrapper::GrumpyCore = new Core();
    CoreWrapper::GrumpyCore->InitCfg();
    CoreWrapper::GrumpyCore->SetSystemEventHandler(new GrumpyEventHandler());
    CoreWrapper::GrumpyCore->InstallFactory(new WidgetFactory());
    InputBox::AE = new AutocompletionEngine();
    MainWindow w;
    w.show();
#ifdef GRUMPY_WIN
    HideConsole();
#endif

    int ReturnCode = a.exec();
    delete CoreWrapper::GrumpyCore;
    return ReturnCode;
}
