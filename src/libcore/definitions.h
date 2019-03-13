//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <QObject>

typedef int user_id_t;
typedef unsigned int scrollback_id_t;

#define GRUMPY_VERSION_STRING "1.0.0"
#define CONFIGURATION_FILE "grumpy.ini"
#define CONFIGURATION_PATH "grumpyirc"

#define GRUMPY_SCRIPT_CONTEXT_CORE            0
#define GRUMPY_SCRIPT_CONTEXT_GRUMPY_CHAT     1
#define GRUMPY_SCRIPT_CONTEXT_GRUMPY_DAEMON   2

#define GRUMPY_EXTENSION_PATH    QCoreApplication::applicationDirPath() + QDir::separator() + "extensions" + QDir::separator()
#define GRUMPY_SCRIPT_PATH       QCoreApplication::applicationDirPath() + QDir::separator() + "scripts" + QDir::separator()

// Default ident for IRC protocol
#define DEFAULT_IDENT "g"
#define PYTHON_ENGINE
#define PRODUCTION_BUILD
#define GRUMPY_PING_HIST 200

#if defined _WIN64 || defined _WIN32
    #define GRUMPY_WIN
#endif
#if QT_VERSION < 0x050000
    #define qintptr int
#endif

#endif // DEFINITIONS_H

