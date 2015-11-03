//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "parser.h"

using namespace irc2htmlcode;

Parser::Parser()
{
    this->TextColor = "#FFFFFF";
    this->TimeColor = "#DCFCAC";
    this->UserColor = "#B4D6FA";
    this->resolveCacheSize = 2000;
    this->cacheHits = 0;
    this->cacheMiss = 0;
}

Parser::~Parser()
{

}

FormattedItem Parser::Process(QString format, QDateTime time, QString user, QString text)
{
    /*if (this->resolveCache.contains(input))
    {
        this->cacheHits++;
        return this->resolveCache[input];
    }
    this->cacheMiss++;*/
    FormattedItem item;
    item.time = time;
    item.text = text;
    item.user = user;

    item.source = this->EncodeHtml(format);
    item.source.replace("$time", this->formatTime(time));
    item.source.replace("$nick", this->formatUser(user));
    item.source.replace("$string", this->formatText(text));

    //item.source = input;
    /*
    // Make some space in cache
    while (this->resolveCache.size() > this->resolveCacheSize)
    {
        // Now we remove the first item, but later we need to figure out these with last access time
        this->resolveCache.remove(this->resolveCache.keys().at(0));
    }

    this->resolveCache.insert(input, item);
    */

    return item;
}

QString Parser::EncodeHtml(QString text)
{
    text.replace("<", "&lt;");
    text.replace(">", "&gt;");

    return text;
}

QString Parser::formatTime(QDateTime time)
{
    return "<font color=\"" + this->TimeColor + "\">" + this->EncodeHtml(time.toString()) + "</font>";
}

QString Parser::formatUser(QString user)
{
    return "<font color=\"" + this->UserColor + "\">" + this->EncodeHtml(user) + "</font>";
}

QString Parser::formatText(QString text)
{
    return "<font color=\"" + this->TextColor + "\">" + this->EncodeHtml(text) + "</font>";
}
