//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include "libcore_global.h"
#include "definitions.h"
#include "../libirc/libirc/serializableitem.h"
#include <QRegExp>
#include <QList>

namespace libircclient
{
    class User;
    class Network;
}

namespace GrumpyIRC
{
    class ScrollbackItem;

    //! \todo No unit test
    class LIBCORESHARED_EXPORT Highlighter : public libirc::SerializableItem
    {
        public:
            //static void Init();
            static bool IsMatch(ScrollbackItem *text, libircclient::Network *network);
            static QList<Highlighter*> Highlighter_Data;

            Highlighter(const QHash<QString, QVariant> &hash);
            Highlighter(const QString &text);
            ~Highlighter() override;
            QHash<QString, QVariant> ToHash() override;
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            bool IsMatching(ScrollbackItem *text, libircclient::Network *network);
            void SetDefinition(const QString &value);
            void MakeRegex();
            QString GetDefinition() const;
            bool CaseSensitive;
            bool Enabled;
            bool Messages;
            bool MatchingSelf;
            bool IsRegex;
        private:
            QString definition;
            QRegExp regex;
    };
}

#endif // HIGHLIGHTER_H
