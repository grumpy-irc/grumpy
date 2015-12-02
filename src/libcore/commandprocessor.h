//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include <QStringList>
#include <QHash>
#include "libcore_global.h"

#define COMMANDPROCESSOR_ENOTEXIST        1
#define COMMANDPROCESSOR_ENOTCONNECTED    2
#define COMMANDPROCESSOR_EEMPTY           3

namespace GrumpyIRC
{
    class Scrollback;
    class SystemCommand;

    class LIBCORESHARED_EXPORT CommandArgs
    {
        public:
            QString ParameterLine;
            QStringList Parameters;
    };

    typedef int (*SC_Callback) (SystemCommand*, CommandArgs);

    class LIBCORESHARED_EXPORT SystemCommand
    {
        public:
            SystemCommand(QString name, SC_Callback callback);
            virtual ~SystemCommand();
            QString GetName();
            int Run(CommandArgs args);
        protected:
            SC_Callback Callback;
        private:
            QString _name;
    };

    class LIBCORESHARED_EXPORT CommandProcessor
    {
        public:
            CommandProcessor();
            ~CommandProcessor();
            void RegisterCommand(SystemCommand *sc);
            //! Simple idiot proof processor for text that is input by a user
            int ProcessText(QString text, Scrollback *window);
            QList<QString> GetCommands();
            bool SplitLong;
            unsigned int LongSize;
            char CommandPrefix;
        private:
            int ProcessItem(QString command, Scrollback *window);
            QHash<QString, SystemCommand*> CommandList;

    };
}

#endif // COMMANDPROCESSOR_H
