//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QDataStream>
#include "generic.h"
#include "exception.h"
#include "networksession.h"
#include "scrollback.h"

using namespace GrumpyIRC;

bool Generic::String2Bool(QString string, bool invalid)
{
    if (string.toLower() == "true")
        return true;
    else if (string.toLower() == "false")
        return false;
    return invalid;
}

QString Generic::Bool2String(bool boolean)
{
    if (boolean)
        return "true";
    return "false";
}

bool GrumpyIRC::Generic::Int2Bool(int integer)
{
    return integer != 0;
}

bool Generic::IsGrumpy(Scrollback *window)
{
    if (!window)
        return false;
    if (window->GetSession())
    {
        return window->GetSession()->GetType() == SessionType_Grumpyd;
    }
    return false;
}

QStringList Generic::Trim(QStringList list)
{
    QStringList result;
    foreach (QString item, list)
        if (!item.isEmpty())
            result << item;
    return result;
}

int GrumpyIRC::Generic::Bool2Int(bool value)
{
    if (value)
        return 1;
    else
        return 0;
}

QByteArray Generic::VariantToByteArray(QVariant data)
{
    QByteArray result;
    QDataStream stream(&result, QIODevice::ReadWrite);
    stream << data;
    return result;
}

QVariant Generic::VariantFromByteArray(QByteArray data)
{
    QDataStream stream(&data, QIODevice::ReadWrite);
    QVariant result;
    stream >> result;
    return result;
}

QHash<QString, QVariant> Generic::MergeHash(QHash<QString, QVariant> x, QHash<QString, QVariant> y)
{
    foreach (QString key, y.keys())
    {
        if (x.contains(key))
            x[key] = y[key];
        else
            x.insert(key, y[key]);
    }
    return x;
}

QList<QVariant> Generic::QStringListToQVariantList(QList<QString> list)
{
    QList<QVariant> results;

    foreach (QString item, list)
        results.append(QVariant(item));

    return results;
}

QList<int> Generic::QVariantListToIntList(QList<QVariant> list)
{
    QList<int> results;

    foreach (QVariant item, list)
        results.append(item.toInt());

    return results;
}

QList<QVariant> Generic::QIntListToVariantList(QList<int> list)
{
    QList<QVariant> results;

    foreach (int item, list)
        results.append(QVariant(item));

    return results;
}

QString Generic::ExpandedString(QString string, unsigned int minimum_size, unsigned int maximum_size)
{
    if (maximum_size != 0 && minimum_size > maximum_size)
        throw new Exception("Maximum size smaller than minimum size", BOOST_CURRENT_FUNCTION);

    if (maximum_size > 0 && static_cast<unsigned int>(string.size()) > maximum_size)
    {
        if (maximum_size < 4)
        {
            string = string.mid(0, maximum_size);
        } else
        {
            string = string.mid(0, maximum_size - 3);
            string += "...";
        }
        return string;
    }

    while (static_cast<unsigned int>(string.size()) < minimum_size)
        string += " ";

    return string;
}

int Generic::LongestString(QList<QString> list)
{
    int longest = 0;
    foreach (QString item, list)
    {
        if (item.size() > longest)
            longest = item.size();
    }
    return longest;
}
