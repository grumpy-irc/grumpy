//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

// This is a base class for most of classes in GrumpyChat, this primarily exists
// for profiler code

#ifndef GRUMPYOBJECT_H
#define GRUMPYOBJECT_H

#include "libcore_global.h"
#include "definitions.h"
#include <QHash>

namespace GrumpyIRC
{
    class GrumpyObject
    {
        public:
            static QHash<QString, quint64> GetClassInstanceCounts();

            GrumpyObject(QString name = "");
            virtual ~GrumpyObject();

        protected:
            void grumpyObjectIncrementCount();
            void setGrumpyObjectName(QString name);

        private:
#ifdef GRUMPY_PROFILER
            static QHash<QString, quint64> grumpyObjectTotalCount;
            QString grumpyObjectName;
#endif
    };
}

#endif // GRUMPYOBJECT_H
