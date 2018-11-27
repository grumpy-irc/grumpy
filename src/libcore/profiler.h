//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef PROFILER_HPP
#define PROFILER_HPP

#ifdef GRUMPY_PROFILER
#include <QDateTime>
#include <QString>
#include <QHash>

#define GRUMPY_PROFILER_RESET GrumpyIRC::Profiler::Reset()
#define GRUMPY_PROFILER_INCRCALL(function) GrumpyIRC::Profiler::IncrementCall(function)
#define GRUMPY_PROFILER_TIME  GrumpyIRC::Profiler::GetTime()
#define GRUMPY_PROFILER_PRINT_TIME(function) GRUMPY_DEBUG(QString("PROFILER: ") + function + " finished in " + QString::number(GrumpyIRC::Profiler::GetTime()) + "ms"); GrumpyIRC::Profiler::Reset()

namespace GrumpyIRC
{
    class Profiler
    {
        public:
            static void Reset();
            static qint64 GetTime();
            static void IncrementCall(QString function);
            static unsigned long long GetCallsForFunction(QString function);
            static QList<QString> GetRegisteredCounterFunctions();
        private:
            static QHash<QString, unsigned long long> callCounter;
            static QDateTime ts;
    };
}
#else

#define GRUMPY_PROFILER_PRINT_TIME(function)
#define GRUMPY_PROFILER_RESET
#define GRUMPY_PROFILER_TIME 0
#define GRUMPY_PROFILER_INCRCALL

#endif

#endif // GRUMPYPROFILER_HPP
