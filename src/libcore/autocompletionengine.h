//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef AUTOCOMPLETIONENGINE_H
#define AUTOCOMPLETIONENGINE_H

#include <QChar>
#include <QList>
#include <QString>

namespace GrumpyIRC
{
    class AutocompletionInformation
    {
        public:
            AutocompletionInformation();
            QString FullText;
            unsigned int Position;
    };

    class AutocompletionEngine
    {
        public:
            AutocompletionEngine();
            virtual ~AutocompletionEngine();
            AutocompletionInformation Execute(AutocompletionInformation input);
        protected:
            QList<QChar> WordSeparators;
    };
}

#endif // AUTOCOMPLETIONENGINE_H
