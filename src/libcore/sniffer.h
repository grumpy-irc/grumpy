//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef SNIFFER_H
#define SNIFFER_H

#include "../libirc/libirc/serializableitem.h"
#include "definitions.h"
#include "libcore_global.h"
#include <QString>
#include <QDateTime>

namespace GrumpyIRC
{
    class LIBCORESHARED_EXPORT NetworkSniffer_Item : public libirc::SerializableItem
    {
        public:
            NetworkSniffer_Item(QByteArray data, bool is_outgoing);
            NetworkSniffer_Item(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash() override;
            void LoadHash(QHash<QString, QVariant> hash) override;
            bool IsOutgoing;
            QDateTime Time;
            QString Text;
    };
}

#endif // SNIFFER_H
