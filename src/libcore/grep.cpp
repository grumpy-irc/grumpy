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
    /*if (!this->regex.startsWith("^") && !this->regex.startsWith(".*"))
        this->regex = ".*" + this->regex;
    if (!this->regex.endsWith(".*") && !this->regex.endsWith("$"))
        this->regex += ".*";*/
    this->caseSensitive = case_sensitive;
}

GREP::~GREP()
{

}

bool GREP::IsValid()
{
    QRegExp regexp(this->regex, Qt::CaseSensitive);
    return regexp.isValid();
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
        // dummy call to fill in all the stuff
        QString original_text = item.GetText();
        if (regexp.indexIn(original_text) != -1)
        {
            if (this->Highlight)
            {
                QStringList strings = regexp.capturedTexts();

                // Highlight each matched part
                foreach (QString matched_text, strings)
                    original_text.replace(matched_text, QString((char)2) + QString((char)3) + "4" + matched_text + QString((char)3) + QString((char)2));

                item.SetText(original_text);
            }
            // we got results, append the context buffer here
            results.append(context_buffer);
            context_buffer.clear();
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
                if (context_buffer.size() >= this->Context)
                    context_buffer.removeFirst();
                context_buffer.append(item);
            }
        }
    }
    return results;
}
