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
#include "grumpyconf.h"
#include "../libcore/exception.h"
#include "grumpyeventhandler.h"
#include "../libcore/autocompletionengine.h"
#include "../libcore/core.h"
#include "../libcore/terminalparser.h"
#include "../libcore/highlighter.h"
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

int Parser_KeepCons(GrumpyIRC::TerminalParser *parser, QStringList params)
{

    return 0;
}

int Parser_Verbosity(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    Q_UNUSED(params);
    Q_UNUSED(parser);
    CONF->Verbosity++;
    return 0;
}

int Parser_SafeMode(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    Q_UNUSED(params);
    (void)parser;

    CONF->SafeMode = true;
    return 0;
}

int main(int argc, char *argv[])
{
    int ReturnCode = 0;
    try
    {
        CONF = new GrumpyConf();
        TerminalParser *tp = new TerminalParser();
        tp->Register('v', "--verbose", "Increase verbosity level", 0, (TP_Callback)Parser_Verbosity);
        tp->Register('k', "--cons", "Keep console on", 0, (TP_Callback)Parser_KeepCons);
        tp->Register('m', "--safe", "Start GrumpyChat in a safe mode", 0, (TP_Callback)Parser_SafeMode);
        if (!tp->Parse(argc, argv))
        {
            delete tp;
            return ReturnCode;
        }
        delete tp;

        QApplication a(argc, argv);
        a.setApplicationName("GrumpyChat");
        a.setOrganizationName("grumpy");
        // Initialize core first
        CoreWrapper::GrumpyCore = new Core();
        CoreWrapper::GrumpyCore->InitCfg();
        GrumpyConf::Conf->Load();
        CoreWrapper::GrumpyCore->SetSystemEventHandler(new GrumpyEventHandler());
        CoreWrapper::GrumpyCore->InstallFactory(new WidgetFactory());
        ScrollbackFrame::InitializeThread();
        InputBox::AE = new AutocompletionEngine();
        MainWindow w;
        w.show();
    #ifdef GRUMPY_WIN
        HideConsole(0);
    #endif
        ReturnCode = a.exec();
        delete CoreWrapper::GrumpyCore;
        delete CONF;
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
