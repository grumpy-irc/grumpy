//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef AUTOCOMPLETIONENGINE_H
#define AUTOCOMPLETIONENGINE_H

#include <QChar>
#include <QList>
#include "libcore_global.h"
#include <QString>

namespace GrumpyIRC
{
    class LIBCORESHARED_EXPORT AutocompletionInformation
    {
        public:
            AutocompletionInformation();
            QString FullText;
            int Position;
            QList<QString> Suggestions;
            bool Success;
    };

    class LIBCORESHARED_EXPORT AutocompletionEngine
    {
        public:
            AutocompletionEngine();
            virtual ~AutocompletionEngine();
            virtual AutocompletionInformation Execute(AutocompletionInformation input, QList<QString> extra_commands, QList<QString> users, QList<QString> channels);
        protected:
            virtual QString replaceWord(QString source, int start, QString sw, QString target);
            virtual QString getIsolatedWord(AutocompletionInformation input, int *start_pos);
            //! This function returns the part of a word that all words in a list share, so for a list of these words:
            //! server_addr server_port server_user
            //! this function returns "server_"
            //! If you already know that all of these words start with something you can provide as a hint so that
            //! it performs faster
            virtual QString getSimilar(QList<QString> words, QString hint = "");
            virtual AutocompletionInformation processList(QList<QString> list_of_words_cmp, bool *success, bool case_sensitive, QString word, int start, QString full_text);
            char channelPrefix;
            QList<QChar> WordSeparators;
    };
}

#endif // AUTOCOMPLETIONENGINE_H
