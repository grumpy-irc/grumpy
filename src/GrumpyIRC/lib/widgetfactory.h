//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include "grumpy_global.h"
#include <libcore/factory.h>

namespace GrumpyIRC
{
    class LIBGRUMPYSHARED_EXPORT WidgetFactory : public Factory
    {
        public:
            WidgetFactory() = default;
            Scrollback *NewScrollback(Scrollback *parent, const QString &name, ScrollbackType type) override;
    };
}

#endif // WIDGETFACTORY_H
