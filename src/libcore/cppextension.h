//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef CPPEXTENSION_H
#define CPPEXTENSION_H

#include "libcore_global.h"
#include "extension.h"
#include <QtPlugin>

namespace GrumpyIRC
{
    class LIBCORESHARED_EXPORT CppExtension : public Extension
    {
        public:
            CppExtension();
            ~CppExtension() override;


        public slots:
    };
}

Q_DECLARE_INTERFACE(GrumpyIRC::CppExtension, "org.grumpy.extension.qt")

#endif // CPPEXTENSION_H
