//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "profiler.h"
#include <algorithm> // Add this include for std::sort

#ifdef GRUMPY_PROFILER
using namespace GrumpyIRC;

QHash<QString, unsigned long long> Profiler::callCounter;
QDateTime Profiler::ts = QDateTime::currentDateTime();

void Profiler::Reset()
{
    ts = QDateTime::currentDateTime();
}

qint64 Profiler::GetTime()
{
    return ts.msecsTo(QDateTime::currentDateTime());
}

void Profiler::IncrementCall(const QString &function)
{
    if (!callCounter.contains(function))
        callCounter.insert(function, 1);
    callCounter[function]++;
}

unsigned long long Profiler::GetCallsForFunction(const QString &function)
{
    if (!callCounter.contains(function))
        return 0;
    return callCounter[function];
}

QList<QString> Profiler::GetRegisteredCounterFunctions()
{
    // sort the list by number of calls
    QHash<QString, unsigned long long> temp = callCounter;
    QList<unsigned long long> Numbers = temp.values();
    std::sort(Numbers.begin(), Numbers.end());
    QList<QString> Functions;
    foreach(unsigned long long n, Numbers)
    {
        QString key = temp.key(n);
        Functions.append(key);
        temp.remove(key);
    }
    return Functions;
}
#endif
