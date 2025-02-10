//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2020

#include <QObject>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QRegularExpression regexp(this->regex);
#else
    QRegExp regexp(this->regex, Qt::CaseSensitive);
#endif
    return regexp.isValid();
}

QList<ScrollbackItem> GREP::Exec(QList<ScrollbackItem> buff)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QRegularExpression regexp(this->regex);
#else
    QRegExp regexp(this->regex, Qt::CaseSensitive);
#endif
    if (!this->caseSensitive)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        regexp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
#else
        regexp.setCaseSensitivity(Qt::CaseInsensitive);
#endif
    QList<ScrollbackItem> results;
    QList<ScrollbackItem> context_buffer;
    int remaining_buffer = 0;
    foreach (ScrollbackItem item, buff)
    {
        // dummy call to fill in all the stuff
        QString original_text = item.GetText();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (regexp.match(original_text).hasMatch())
#else
        if (regexp.indexIn(original_text) != -1)
#endif
        {
            if (this->Highlight)
            {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QStringList strings = regexp.match(original_text).capturedTexts();
#else
                QStringList strings = regexp.capturedTexts();
#endif

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
        } 
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        else if (regexp.match(item.GetUser().GetNick()).hasMatch())
#else
        else if (this->MatchNick && regexp.indexIn(item.GetUser().GetNick()) != -1)
#endif
        {
            // Nickname matches the regex
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
