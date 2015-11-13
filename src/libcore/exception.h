//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QList>
#include <QString>
#include "libcore_global.h"

#ifdef BOOST_CURRENT_FUNCTION
    #undef BOOST_CURRENT_FUNCTION
#endif

// stolen from boost/current_function.hpp
// enhanced by Petr Bena, keep the enhancement in case you wanted to update this

#ifndef __LINE__
    #define __LINE__ 0
#endif

#ifndef __FILE__
    #define __FILE__ "{unknown}"
#endif

#ifdef GetMessage
    #undef GetMessage
#endif

#define HUGGLE_FILE_LINE QString("@") + QString(__FILE__) + QString(":") + QString::number(__LINE__)

#ifndef BOOST_CURRENT_FUNCTION
    #if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600))
        #define BOOST_CURRENT_FUNCTION QString(__PRETTY_FUNCTION__) + HUGGLE_FILE_LINE
    #elif defined(__DMC__) && (__DMC__ >= 0x810)
        #define BOOST_CURRENT_FUNCTION QString(__PRETTY_FUNCTION__) + HUGGLE_FILE_LINE
    #elif defined(__FUNCSIG__)
        #define BOOST_CURRENT_FUNCTION QString(__FUNCSIG__) + HUGGLE_FILE_LINE
    #elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
        #define BOOST_CURRENT_FUNCTION QString(__FUNCTION__) + HUGGLE_FILE_LINE
    #elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
        #define BOOST_CURRENT_FUNCTION QString(__FUNC__) + HUGGLE_FILE_LINE
    #elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
        #define BOOST_CURRENT_FUNCTION QString(__func__) + HUGGLE_FILE_LINE
    #else
        #define BOOST_CURRENT_FUNCTION QString("(unknown)") + HUGGLE_FILE_LINE
    #endif
#endif

namespace GrumpyIRC
{
    class LIBCORESHARED_EXPORT Exception
    {
        public:
            Exception();
            Exception(QString Message, QString function_id);

            virtual ~Exception() {}
            virtual int ErrorCode() { return errc; }
            virtual QString GetMessage();
            virtual QString GetSource();
        private:
            int errc;
            QString source;
            QString message;
            QString stack_tree;
    };

    class LIBCORESHARED_EXPORT NullPointerException : public Exception
    {
        public:
            NullPointerException(QString pointer, QString signature);
    };
}

#endif // EXCEPTION_H
