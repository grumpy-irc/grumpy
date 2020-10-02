//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2020

#ifndef GENERIC_H
#define GENERIC_H

#include <QStringList>
#include <QString>
#include <QObject>
#include "libcore_global.h"
#include <QList>

#ifdef MessageBox
    #undef MessageBox
#endif

namespace GrumpyIRC
{
    class Scrollback;

    namespace Generic
    {
        class HostInfo
        {
            public:
                QString Host;
                int Port = 0;
                bool Invalid = false;
        };

        enum MessageBox_Type
        {
            MessageBox_Type_Normal,
            MessageBox_Type_Question,
            MessageBox_Type_Warning,
            MessageBox_Type_Error
        };

        LIBCORESHARED_EXPORT bool String2Bool(const QString &string, bool invalid = false);
        LIBCORESHARED_EXPORT QString Bool2String(bool boolean);
        LIBCORESHARED_EXPORT bool IsValidFileName(QString file);
        //! \todo No unit test
        LIBCORESHARED_EXPORT QStringList Trim(QStringList list);
        LIBCORESHARED_EXPORT int Bool2Int(bool value);
        LIBCORESHARED_EXPORT bool Int2Bool(int integer);
        //! Returns true if this scrollback is belonging to grumpy server, instead of local instance
        LIBCORESHARED_EXPORT bool IsGrumpyd(Scrollback *window);
        //! \todo No unit test
        LIBCORESHARED_EXPORT QHash<QString, QVariant> MergeHash(QHash<QString, QVariant> &x, const QHash<QString, QVariant> &y);
        //! \todo No unit test
        LIBCORESHARED_EXPORT QByteArray VariantToByteArray(const QVariant &data);
        //! \todo No unit test
        LIBCORESHARED_EXPORT QVariant VariantFromByteArray(QByteArray &data);
        LIBCORESHARED_EXPORT QList<QVariant> QStringListToQVariantList(const QList<QString> &list);
        LIBCORESHARED_EXPORT QList<int> QVariantListToIntList(const QList<QVariant> &list);
        LIBCORESHARED_EXPORT QString GetResource(const QString &name);
        LIBCORESHARED_EXPORT QString StripSpecial(QString text);
        LIBCORESHARED_EXPORT QList<QVariant> QIntListToVariantList(const QList<int> &list);
        LIBCORESHARED_EXPORT QString ExpandedString(QString string, unsigned int minimum_size, unsigned int maximum_size = 0);
        LIBCORESHARED_EXPORT int LongestString(const QList<QString> &list);
        LIBCORESHARED_EXPORT QString DoubleDigit(int digit);
        LIBCORESHARED_EXPORT HostInfo GetHostPortInfo(QString target, int default_port);
        LIBCORESHARED_EXPORT bool SecondsToTimeSpan(int time, int *days, int *hours, int *minutes, int *seconds);
    }
}


#endif // GENERIC_H
