//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "grumpyobject.h"
#include "exception.h"

using namespace GrumpyIRC;

#ifdef GRUMPY_PROFILER
QHash<QString, quint64> GrumpyObject::grumpyObjectTotalCount;
#endif

QHash<QString, quint64> GrumpyObject::GetClassInstanceCounts()
{
#ifndef GRUMPY_PROFILER
    return QHash<QString, quint64>();
#else
    return grumpyObjectTotalCount;
#endif
}

GrumpyObject::GrumpyObject(QString name)
{
#ifdef GRUMPY_PROFILER
    if (name.isEmpty())
        return;
    this->grumpyObjectName = name;
    this->grumpyObjectIncrementCount();
#else
    Q_UNUSED(name);
#endif
}

GrumpyObject::~GrumpyObject()
{
#ifdef GRUMPY_PROFILER
    if (this->grumpyObjectName.isEmpty())
        return;
    if (grumpyObjectTotalCount[this->grumpyObjectName] == 0)
        throw new Exception("Negative instance count", BOOST_CURRENT_FUNCTION);
    grumpyObjectTotalCount[this->grumpyObjectName]--;
#endif
}

void GrumpyObject::grumpyObjectIncrementCount()
{
#ifdef GRUMPY_PROFILER
    if (!grumpyObjectTotalCount.contains(this->grumpyObjectName))
        grumpyObjectTotalCount.insert(this->grumpyObjectName, 1);
    else
        grumpyObjectTotalCount[this->grumpyObjectName]++;
#endif
}

void GrumpyObject::setGrumpyObjectName(QString name)
{
#ifdef GRUMPY_PROFILER
    this->grumpyObjectName = name;
#else
    Q_UNUSED(name);
#endif
}
