//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef PARSER_H
#define PARSER_H

#include "libirc2htmlcode_global.h"
#include "formatteditem.h"
#include <QHash>

namespace irc2htmlcode
{
    class LIBIRC2HTMLCODESHARED_EXPORT Parser
    {
        public:
            Parser();
            virtual ~Parser();
            virtual FormattedItem Process(QString format, QDateTime time, QString user, QString text);
            QString UserColor;
            QString TimeColor;
            QString TextColor;
            QHash<unsigned int, QString> TextColors;
            QString EncodeHtml(QString text);

        private:
            QString replaceSpecials(QString source);
            QString formatTime(QDateTime time);
            QString formatUser(QString user);
            QString formatText(QString text);
            int resolveCacheSize;
            unsigned long long cacheHits;
            unsigned long long cacheMiss;
            QHash<QString, FormattedItem> resolveCache;
    };
}

#endif // PARSER_H
