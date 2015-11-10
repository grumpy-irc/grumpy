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

typedef unsigned int user_id_t;
typedef unsigned int scrollback_id_t;

#define GRUMPY_VERSION_STRING "1.0.0"
#define CONFIGURATION_FILE "grumpy.ini"
#define CONFIGURATION_PATH "grumpyirc"
#define PRODUCTION_BUILD

#if defined _WIN64 || defined _WIN32
    #define GRUMPY_WIN
#endif
#if QT_VERSION < 0x050000
    #define qintptr int
#endif

#endif // DEFINITIONS_H

