//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2021

#ifndef IDENTITY_H
#define IDENTITY_H

#include <QString>
#include "libcore_global.h"
#include "../libirc/libirc/serializableitem.h"

namespace GrumpyIRC
{
    class LIBCORESHARED_EXPORT Identity : public libirc::SerializableItem
    {
        public:
            static int LastID;

            Identity(QString nick, QString ident, QString real_name, QString away_msg = "", int id = -1);
            QHash<QString, QVariant> ToHash() override;
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            int ID;
            QString Nick;
            QString Ident;
            QString RealName;
            QString AwayMessage;
    };
}

#endif // IDENTITY_H
