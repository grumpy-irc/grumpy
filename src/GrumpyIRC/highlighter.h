//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include "../libcore/definitions.h"
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

    class Highlighter
    {
        public:
            static void Init();
            static bool IsMatch(ScrollbackItem *text, libircclient::Network *network);
            static QList<Highlighter*> Highlighter_Data;

            Highlighter(QString text);
            ~Highlighter();
            bool IsMatching(ScrollbackItem *text, libircclient::Network *network);
            bool CaseSensitive;
            bool Messages;
            bool MatchingSelf;
            bool IsRegex;
        private:
            QString definition;
            QRegExp regex;
    };
}

#endif // HIGHLIGHTER_H
