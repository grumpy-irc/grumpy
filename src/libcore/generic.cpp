//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "definitions.h"

#include <QDataStream>
#include <QFile>
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

QString Generic::GetResource(QString name)
{
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly))
        throw new Exception("Unable to open internal resource: " + name, BOOST_CURRENT_FUNCTION);

    return QString(file.readAll());
}


QString Generic::StripSpecial(QString text)
{
    text = text.replace((char)1, "").replace((char)2, "").replace((char)4, "").replace((char)15, "").replace((char)16, "").replace((char)29, "");
    // now find a remove color codes
    while (text.contains((char)3))
    {
        int pos = text.indexOf((char)3);
        if (pos + 1 == text.size())
        {
            text.remove(pos, 1);
        } else if (pos + 2 == text.size())
        {
            // let's check if the characted after is a number
            QChar last = text.at(pos + 1);
            if (last.isNumber())
                text.remove(pos, 2);
            else
                text.remove(pos, 1);
        } else
        {
            int remove = 1;
            QChar first = text.at(pos + 1);
            if (first.isNumber())
            {
                remove++;
                QChar second = text.at(pos + 2);
                if (second.isNumber())
                    remove++;
            }
            text.remove(pos, remove);
        }
    }
    return text;
}


Generic::HostInfo Generic::GetHostPortInfo(QString target, int default_port)
{
    target = target.trimmed();
    Generic::HostInfo info;
    info.Port = default_port;
    info.Host = target;

    if (!target.contains(":"))
    {
        return info;
    }

    int p;
    bool success = true;

    // Detect IPv6: [::1]:50
    if (target.startsWith("["))
    {
        // This could be IPv6
        QString ip = target.mid(1);
        if (!ip.contains("]"))
        {
            info.Invalid = true;
            return info;
        }

        // Let's extract IPv6
        ip = ip.mid(0, ip.indexOf("]"));
        info.Host = ip;

        // Now get port if there is any
        QString port = target.mid(target.indexOf("]") + 1);
        if (port.isEmpty())
        {
            // there is no port
            return info;
        }

        p = port.toInt(&success);
        if (!success)
        {
            info.Invalid = true;
            return info;
        }
        info.Port = p;
        return info;
    }

    // This is either IPv4 or string
    info.Host = target.mid(0, target.indexOf(":"));
    p = target.mid(target.indexOf(":") + 1).toInt(&success);
    if (!success)
    {
        info.Invalid = true;
        return info;
    }
    info.Port = p;
    return info;
}
