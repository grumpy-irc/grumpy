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

#include <iostream>
#ifdef GRUMPY_WIN
    #include <windows.h>
#endif
#include "mainwindow.h"
#include "corewrapper.h"
#include "../libcore/exception.h"
#include "grumpyeventhandler.h"
#include "../libcore/autocompletionengine.h"
#include "../libcore/core.h"
#include "highlighter.h"
#include "scrollbackframe.h"
#include "inputbox.h"
#include "widgetfactory.h"
#include <QApplication>

using namespace GrumpyIRC;

#ifdef GRUMPY_WIN
// Normally we would compile this program so it has no console window, but because we are hackers, we want to see a boot log from its startup
// this function hides the console once the grumpy is started up and main window is loaded

// it doesn't do anything if grumpy is compiled in debug mode, because we want to see it all time in that case :)
void HideConsole(int hide)
{
#ifndef _DEBUG
    HWND Stealth;
    AllocConsole();
    Stealth = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(Stealth, hide);
#endif
}
#endif

int main(int argc, char *argv[])
{
    int ReturnCode = 0;
    try
    {
        QApplication a(argc, argv);

        // Initialize core first
        CoreWrapper::GrumpyCore = new Core();
        CoreWrapper::GrumpyCore->InitCfg();
        CoreWrapper::GrumpyCore->SetSystemEventHandler(new GrumpyEventHandler());
        CoreWrapper::GrumpyCore->InstallFactory(new WidgetFactory());
        Highlighter::Init();
        ScrollbackFrame::InitializeThread();
        InputBox::AE = new AutocompletionEngine();
        MainWindow w;
        w.show();
    #ifdef GRUMPY_WIN
        HideConsole(0);
    #endif
        ReturnCode = a.exec();
        delete CoreWrapper::GrumpyCore;
        return ReturnCode;
    } catch (GrumpyIRC::Exception *ex)
    {
        std::cerr << "Unhandled exception: " << ex->GetMessage().toStdString() << std::endl;
        std::cerr << "Source: " << ex->GetSource().toStdString() << std::endl;
        delete ex;
#ifdef GRUMPY_WIN
        HideConsole(1);
        std::cerr << "Press any key to exit grumpy" << std::endl;
        std::cin.get();
#endif
    }
    return ReturnCode;
}
