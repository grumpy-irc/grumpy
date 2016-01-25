//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "gdeventhandler.h"
#include <iostream>
#include "grumpyconf.h"
#include "corewrapper.h"
#include "../libcore/core.h"
#include "../libcore/configuration.h"
#ifdef __linux__
  #include <syslog.h>
#endif

using namespace GrumpyIRC;

GDEventHandler::GDEventHandler()
{

}
#ifdef __linux__
void GDEventHandler::OnDebug(QString text, unsigned int verbosity)
{
    if (GCFG->Verbosity >= verbosity)
    {
        if (CONF->Stdout)
        {
            std::cout << "DEBUG: " << text.toStdString() << std::endl;
            return;
        }
        openlog ("grumpyd", LOG_PID | LOG_DAEMON, 0);
        syslog (LOG_DEBUG, "%s", text.toStdString().c_str());
        closelog ();
    }
}

void GDEventHandler::OnError(QString text)
{
    if (!CONF->Stdout)
    {
        openlog ("grumpyd", LOG_PID | LOG_DAEMON, 0);
        syslog (LOG_ERR, "%s", text.toStdString().c_str());
        closelog ();
    } else
    {
        std::cerr << "ERROR: " << text.toStdString() << std::endl;
        return;
    }
}

void GDEventHandler::OnSystemLog(QString text)
{
    if (CONF->Stdout)
    {
        std::cout << text.toStdString() << std::endl;
    } else
    {
        openlog ("grumpyd", LOG_PID | LOG_DAEMON, 0);
        syslog (LOG_INFO, "%s", text.toStdString().c_str());
        closelog ();
    }
}
#endif
