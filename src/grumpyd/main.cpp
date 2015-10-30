//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QCoreApplication>
//#include <iostream>
#include "corewrapper.h"
#include "scrollbackfactory.h"
#include "../libcore/exception.h"
#include "grumpyconf.h"
#include "grumpyd.h"
#include "../libcore/configuration.h"
#include "../libcore/definitions.h"
#include "../libcore/core.h"
#include "../libcore/terminalparser.h"
#include "../libcore/eventhandler.h"
#include "../libcore/exception.h"
#include "../libcore/generic.h"

#ifdef __linux__
#include <proc/readproc.h>
#endif

#define DAEMONIZE_SUCCESS 0
#define DAEMONIZE_FORKED  1
#define DAEMONIZE_FAILED  2
#define EDAEMONIZEFAILED    -1
#define EUNHANDLEDEXCEPTION -2

int daemonize()
{
    if (!CONF->Daemon)
        return DAEMONIZE_SUCCESS;

    // Platform specific
#ifdef __linux__
    pid_t pid = fork();

    if (pid < 0)
    {
        GRUMPY_ERROR("Failed to daemonize itself, call to fork() failed with error code: " + QString::number(errno));
        return DAEMONIZE_FAILED;
    }

    if (pid > 0)
    {
        GRUMPY_LOG("Parent: forked to pid: " + QString::numer(pid));
        return DAEMONIZE_FORKED;
    }
    return DAEMONIZE_SUCCESS;
#endif
}

int main(int argc, char *argv[])
{
    try
    {
        // This is just a wrapper around libcore configuration system so that we can easily access some global config options
        CONF = new GrumpyIRC::GrumpyConf();
        // First of all we need to process the arguments and then do other stuff
        GrumpyIRC::TerminalParser *tp = new GrumpyIRC::TerminalParser();
        if (!tp->Parse(argc, argv))
        {
            // We processed some argument which requires the application to exit
            delete tp;
            return 0;
        }
        delete tp;

        // Handle daemonizing as system service
        int dr = daemonize();
        if (dr == DAEMONIZE_FAILED)
            return EDAEMONIZEFAILED;
        if (dr == DAEMONIZE_FORKED)
            return 0;

        GrumpyIRC::CoreWrapper::GrumpyCore = new GrumpyIRC::Core();
        // Install our own scrollback factory that creates scrollbacks which are automagically network synced
        GrumpyIRC::CoreWrapper::GrumpyCore->InstallFactory(new GrumpyIRC::ScrollbackFactory());
        GrumpyIRC::CoreWrapper::GrumpyCore->InitCfg();
        // Save the configuration immediately so that we have the configuration file
        GrumpyIRC::CoreWrapper::GrumpyCore->GetConfiguration()->Save();
        GRUMPY_LOG("Grumpyd starting...");
        GrumpyIRC::Grumpyd *daemon = new GrumpyIRC::Grumpyd();
        QTimer::singleShot(0, daemon, SLOT(Main()));
        QCoreApplication a(argc, argv);
        return a.exec();
    } catch (GrumpyIRC::Exception *exception)
    {
        GRUMPY_ERROR("FATAL: " + exception->GetMessage());
        return EUNHANDLEDEXCEPTION;
    }
}

