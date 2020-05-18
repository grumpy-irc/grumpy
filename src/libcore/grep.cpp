//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2020

#include <QRegExp>
#include "grep.h"

using namespace GrumpyIRC;

GREP::GREP(QString regex, bool case_sensitive)
{
    this->regex = regex;
    this->caseSensitive = case_sensitive;
}

GREP::~GREP()
{

}

QList<ScrollbackItem> GREP::Exec(QList<ScrollbackItem> buff)
{
    QRegExp regexp(this->regex, Qt::CaseSensitive);
    if (!this->caseSensitive)
        regexp.setCaseSensitivity(Qt::CaseInsensitive);
    QList<ScrollbackItem> results;
    QList<ScrollbackItem> context_buffer;
    int remaining_buffer = 0;
    foreach (ScrollbackItem item, buff)
    {
        if (regexp.exactMatch(item.GetText()))
        {
            // we got results, append the context buffer here
            results.append(context_buffer);
            results.append(item);
            remaining_buffer = this->Context;
        } else
        {
            if (remaining_buffer > 0)
            {
                remaining_buffer--;
                results.append(item);
            } else
            {
                if (context_buffer.size() > this->Context)
                    context_buffer.removeFirst();
                context_buffer.append(item);
            }
        }
    }
    return results;
}
