//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifdef QT_GUI_LIB
#include <QMessageBox>
#endif
#include <QDataStream>
#include "generic.h"
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

int Generic::MessageBox(QString title, QString message, GrumpyIRC::Generic::MessageBox_Type type, QObject *parent)
{
#ifdef QT_GUI_LIB
    QMessageBox *mb = new QMessageBox((QWidget*)parent);
    mb->setAttribute(Qt::WA_DeleteOnClose);
    mb->setText(message);
    mb->setWindowTitle(title);

    switch (type)
    {
        case MessageBox_Type_Error:
            mb->setIcon(QMessageBox::Critical);
            break;
        case MessageBox_Type_Normal:
            mb->setIcon(QMessageBox::Information);
            break;
        case MessageBox_Type_Question:
            mb->setIcon(QMessageBox::Question);
            break;
        case MessageBox_Type_Warning:
            mb->setIcon(QMessageBox::Warning);
            break;
    }

    mb->show();
#else
    // print to cout
#endif
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
