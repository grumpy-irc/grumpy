//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include <QStringList>
#include <QHash>
#include "libcore_global.h"

#define COMMANDPROCESSOR_ENOTEXIST        1
#define COMMANDPROCESSOR_ENOTCONNECTED    2
#define COMMANDPROCESSOR_EEMPTY           3
#define COMMANDPROCESSOR_ETOOMANYREDS     20

namespace GrumpyIRC
{
    class Scrollback;
    class SystemCommand;

    class LIBCORESHARED_EXPORT CommandArgs
    {
        public:
            QString ParameterLine;
            QStringList Parameters;
            //! Scrollback from which this command was executed
            Scrollback *SrcScrollback;
    };

    typedef int (*SC_Callback) (SystemCommand*, CommandArgs);

    class LIBCORESHARED_EXPORT SystemCommand
    {
        public:
            SystemCommand(QString name, SC_Callback callback);
            virtual ~SystemCommand();
            QString GetName();
            virtual int Run(CommandArgs args);
        protected:
            SC_Callback Callback;
        private:
            QString _name;
    };

    /*!
     * \brief The CommandProcessor class provides the universal processor of IRC commands
     *
     * Basically all input from user should be handled using this class
     */
    class LIBCORESHARED_EXPORT CommandProcessor
    {
        public:
            CommandProcessor();
            ~CommandProcessor();
            //! Registers a new command to command processor, it is required by whoever registers the command to verify that command with same name isn't already registered
            void RegisterCommand(SystemCommand *sc);
            void UnregisterCommand(SystemCommand *sc);
            //! Handles a single item - can process only 1 line of text
            int ProcessItem(QString command, Scrollback *scrollback);
            //! Simple idiot proof processor for text that is input by a user
            int ProcessText(QString text, Scrollback *scrollback, bool comments_rm = false);
            QList<QString> GetCommands();
            QHash<QString, QString> GetAliasRdTable();
            QList<QString> GetAList();
            bool Exists(QString name) const;
            void RegisterAlias(QString name, QString target);
            void UnregisterAlias(QString name);
            //! Whether long text message should be split
            bool SplitLong;
            bool AutoReduceMsgSize = true;
            //! How many characters is a long message
            int LongSize;
            //! Minimal size of message to send when splitting a long message, this is needed to prevent delivery of short messages
            //! that would contain just one short word
            int MinimalSize;
            //! Prefix symbol for commands
            char CommandPrefix;
            //! Prefix for comments for script processor
            char CommentChar;
        private:
            QHash<QString, QString> aliasList;
            QHash<QString, SystemCommand*> commandList;
    };
}

#endif // COMMANDPROCESSOR_H
