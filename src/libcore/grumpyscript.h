//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef GRUMPYSCRIPT_H
#define GRUMPYSCRIPT_H

#include "definitions.h"
#include <QHash>

namespace GrumpyIRC
{
    class LIBCORESHARE_EXPORT GrumpyScript
    {
        public:
            static QString ReplaceVars(QString source, QHash<QString, QString> vars, char prefix = '$', char escape = '\\');
            static QString ReplaceStdVars(QString text);

            GrumpyScript();
            //virtual ~GrumpyScript()
            void Exec();

        private:
            QString _source;
    };
}

#endif // GRUMPYSCRIPT_H
