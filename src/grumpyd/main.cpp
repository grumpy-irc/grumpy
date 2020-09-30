//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <csignal>
#include <iostream>
#include "corewrapper.h"
#include "scrollbackfactory.h"
#include "gdeventhandler.h"
#include "grumpyconf.h"
#include "grumpyd.h"
#include "../libcore/configuration.h"
#include "../libcore/core.h"
#include "../libcore/definitions.h"
#include "../libcore/eventhandler.h"
#include "../libcore/exception.h"
#include "../libcore/terminalparser.h"
#include "../libcore/generic.h"
#include <errno.h>

#ifdef __linux__
#include <unistd.h>
#endif

#define DAEMONIZE_SUCCESS 0
#define DAEMONIZE_FORKED  1
#define DAEMONIZE_FAILED  2
#define EDAEMONIZEFAILED    -1
#define EUNHANDLEDEXCEPTION -2
#define EPIDMKER            -3

//////////////////////////////////////////////////////////////////
// Terminal parameters

int verbosity = 0;
QString config_file;

void err(QString text)
{
    std::cerr << "ERROR: " << text.toStdString() << std::endl;
}

int daemonize()
{
    if (!CONF->Daemon)
        return DAEMONIZE_SUCCESS;

    // Platform specific
#ifdef __linux__
    pid_t pid = fork();

    if (pid < 0)
    {
        err("Failed to daemonize itself, call to fork() failed with error code: " + QString::number(errno));
        return DAEMONIZE_FAILED;
    }

    if (pid > 0)
    {
        GRUMPY_LOG("Parent: forked to pid: " + QString::number(pid));
        return DAEMONIZE_FORKED;
    }
#endif
    return DAEMONIZE_SUCCESS;
}

int upgrade_store(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    Q_UNUSED(params);
    Q_UNUSED(parser);
    
    // Flag the db upgrades for later
    CONF->Upgrade = true;

    return TP_RESULT_OK;
}

int service(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    Q_UNUSED(parser);
    Q_UNUSED(params);
    CONF->Daemon = true;

    return TP_RESULT_OK;
}

//! This will enable the dummy database if user wants that
int dummy(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    Q_UNUSED(params);
    Q_UNUSED(parser);
    CONF->StorageDummy = true;

    return TP_RESULT_OK;
}

int verbosity_plus(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    Q_UNUSED(params);
    Q_UNUSED(parser);

    // Increase the verbosity of whole grumpy
    verbosity++;
    return TP_RESULT_OK;
}

int config(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    Q_UNUSED(parser);
    if (params.isEmpty())
    {
        err("Invalid file");
        return TP_RESULT_SHUT;
    }

    config_file = params[0];

    return TP_RESULT_OK;
}

int log_stdout(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    (void)parser;

    // Flag here that we want to use
    // stdout for logs
    CONF->Stdout = true;

    return TP_RESULT_OK;
}

int pid(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    (void)parser;
    if (params.isEmpty())
    {
        err("Invalid pid file");
        return TP_RESULT_SHUT;
    }
    CONF->PID = params[0];
    return TP_RESULT_OK;
}

int dbcl(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    (void)params;
    (void)parser;
    CONF->AutoFix = true;
    return TP_RESULT_OK;
}

int migrate(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    (void)params;
    (void)parser;
    CONF->DBMove = true;
    return TP_RESULT_SHUT;
}

int trim(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    (void)parser;
    CONF->DBTrim = true;
    CONF->DBMaint = true;
    return TP_RESULT_OK;
}

int default_port(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    (void)parser;
    if (params.isEmpty())
    {
        err("Missing port number");
        return TP_RESULT_SHUT;
    }
    bool processed = true;
    int port = params[0].toInt(&processed);
    if (!processed || port < 0)
    {
        err("Invalid port number");
        return TP_RESULT_SHUT;
    }
    CONF->DefaultPort = port;
    return TP_RESULT_OK;
}

int secured_port(GrumpyIRC::TerminalParser *parser, QStringList params)
{
    (void)parser;
    if (params.isEmpty())
    {
        err("Missing port number");
        return TP_RESULT_SHUT;
    }
    bool processed = true;
    int port = params[0].toInt(&processed);
    if (!processed || port < 0)
    {
        err("Invalid port number");
        return TP_RESULT_SHUT;
    }
    CONF->SecuredPort = port;
    return TP_RESULT_OK;
}

//////////////////////////////////////////////////////////////////
// Signal handler

void grumpyd_terminate()
{
    // Stop all sessions
    GrumpyIRC::Grumpyd::grumpyd->Kill();

    // Delete instance of grumpyd first
    delete GrumpyIRC::Grumpyd::grumpyd;

    // Delete the core, that should handle most of the memory deallocation
    delete GrumpyIRC::CoreWrapper::GrumpyCore;
}

void signal_handler(int sn)
{
    GRUMPY_LOG("Received signal: " + QString::number(sn));
    GRUMPY_LOG("Exiting");
    grumpyd_terminate();
    exit(0);
}

bool safe_mkdir(QString path)
{
    if (!QDir().exists(path))
    {
        GRUMPY_DEBUG("Creating path: " + path, 1);
        // Let's try to make it
        // We might fail here
        if (!QDir().mkpath(path))
        {
            GRUMPY_ERROR("Unable to create path: " + path);
            return false;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////
// Main

int main(int argc, char *argv[])
{
    try
    {
        QCoreApplication a(argc, argv);
        QCoreApplication::setApplicationName("grumpyd");
        // This is just a wrapper around libcore configuration system so that we can easily access some global config options
        CONF = new GrumpyIRC::GrumpyConf();

        // First of all we need to process the arguments and then do other stuff
        GrumpyIRC::TerminalParser *tp = new GrumpyIRC::TerminalParser();
        tp->Register('x', "pid", "Write process ID to a file", 1, (GrumpyIRC::TP_Callback)pid);
        tp->Register('d', "daemonize", "Will start grumpyd as system service", 0, (GrumpyIRC::TP_Callback)service);
        tp->Register('c', "config", "Specify a path to configuration file", 1, (GrumpyIRC::TP_Callback)config);
        tp->Register(0, "dummy", "Use dummy as a storage backend, useful for debugging.", 0, (GrumpyIRC::TP_Callback)dummy);
        tp->Register(0, "cleanup", "Remove invalid objects from the database permanently.", 0, (GrumpyIRC::TP_Callback)dbcl);
        tp->Register(0, "upgrade-db", "Upgrade the database schemas. Required only if grumpyd asks for it.", 0, (GrumpyIRC::TP_Callback)upgrade_store);
        tp->Register(0, "trim-db", "Removes items older than 10 days from scrollback buffers", 0, (GrumpyIRC::TP_Callback)trim);
        tp->Register('v', "verbosity", "Increases the verbose level", 0, (GrumpyIRC::TP_Callback)verbosity_plus);
        tp->Register('s', "log-stdout", "Use current tty for logging instead of syslog", 0, (GrumpyIRC::TP_Callback)log_stdout);
        tp->Register('p', "port", "Change the listener port", 1, (GrumpyIRC::TP_Callback)default_port);
        tp->Register('w', "secured-port", "Change the SSL port", 1, (GrumpyIRC::TP_Callback)secured_port);
        tp->Register('m', "migrate", "Migrate from SQLite to PostgreSQL", 0, (GrumpyIRC::TP_Callback)migrate);

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

        // Write pid
        if (!CONF->PID.isEmpty())
        {
            QFile pf(CONF->PID);
            if (!pf.open(QIODevice::WriteOnly))
            {
                err("Unable to write to pid file: " + CONF->PID);
                return EPIDMKER;
            }
            pf.write(QString::number(QCoreApplication::applicationPid()).toLatin1());
        }

        // Register signals
        signal(SIGTERM, signal_handler);
        signal(SIGINT, signal_handler);

        GrumpyIRC::CoreWrapper::GrumpyCore = new GrumpyIRC::Core();

        // Install our own scrollback factory that creates scrollbacks which are automagically network synced
        GrumpyIRC::CoreWrapper::GrumpyCore->SetSystemEventHandler(new GrumpyIRC::GDEventHandler());

        // Event handler does logging too so now we can write to logs
        GRUMPY_LOG("Grumpyd starting...");
        GRUMPY_LOG("Version: " + QString(GRUMPY_VERSION_STRING) + " Qt: " + QString(QT_VERSION_STR) + "/" + QString(qVersion()));
        GrumpyIRC::CoreWrapper::GrumpyCore->InstallFactory(new GrumpyIRC::ScrollbackFactory());
        GrumpyIRC::CoreWrapper::GrumpyCore->InitCfg();
        if (!config_file.isEmpty())
            CONF->GetConfiguration()->SetAlternativeConfigFile(config_file);
        GrumpyIRC::CoreWrapper::GrumpyCore->LoadCfg();
        GrumpyIRC::Core::GrumpyCore->GetConfiguration()->Verbosity = verbosity;
        // Save the configuration immediately so that we have the configuration file
        CONF->SetStorage(CONF->GetStorage());
        CONF->SetDatafilePath(CONF->GetDatafilePath());
        CONF->SetCertFilePath(CONF->GetCertFilePath());
        CONF->SetScriptPath(CONF->GetScriptPath());
        GrumpyIRC::CoreWrapper::GrumpyCore->GetConfiguration()->Save();
        GRUMPY_LOG("Datafile path: " + CONF->GetDatafilePath());
        // Check if the datafile path exists
        if (!safe_mkdir(CONF->GetDatafilePath()) || !safe_mkdir(CONF->GetScriptPath()))
            return 1;
        GrumpyIRC::Grumpyd *daemon = new GrumpyIRC::Grumpyd();
        QTimer::singleShot(0, daemon, SLOT(Main()));
        GRUMPY_DEBUG("Verbosity level: " + QString::number(verbosity), 1);
        return a.exec();
    } catch (GrumpyIRC::Exception *exception)
    {
        GRUMPY_ERROR("FATAL: " + exception->GetMessage());
        return EUNHANDLEDEXCEPTION;
    }
}

