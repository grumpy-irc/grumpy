//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef USERFRAMEITEM_H
#define USERFRAMEITEM_H

#include "grumpy_global.h"
#include <libcore/grumpyobject.h>
#include <QListWidgetItem>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QColor>
#endif

namespace libircclient
{
    class Network;
}

namespace GrumpyIRC
{
    class LIBGRUMPYSHARED_EXPORT UserFrameItem : public QListWidgetItem, public GrumpyObject
    {
        public:
            UserFrameItem(const QString &text, libircclient::Network *nt);
            bool operator<(const QListWidgetItem &other) const override;
            bool lowerThan(const QListWidgetItem &other) const;
            #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            void setTextColor(const QColor &color);
            #endif
            libircclient::Network *network;
    };
}

#endif // USERFRAMEITEM_H
