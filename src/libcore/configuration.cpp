//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "configuration.h"
#include <QStringList>
#include <QDebug>
#include <QList>
#include <QSettings>

using namespace GrumpyIRC;

QString Configuration::GetVersion()
{
    return GRUMPY_VERSION_STRING;
}

Configuration::Configuration()
{
    this->Verbosity = 60;
}

Configuration::~Configuration()
{
    this->Options.clear();
}

QVariant Configuration::GetValue(QString key)
{
    if (!this->Options.contains(key))
        return QVariant();

    return this->Options[key];
}

void Configuration::SetAlternativeConfigFile(QString file)
{
    this->configuration_path = file;
}

bool Configuration::GetValueAsBool(QString key, bool none)
{
    if (!this->Options.contains(key))
        return none;

    return this->Options[key].toBool();
}

void Configuration::RemoveValue(QString key)
{
    if (!this->Options.contains(key))
    {
        return;
    }

    this->Options.remove(key);
}

QString Configuration::GetValueAsString(QString key, QString default_value)
{
    if (!this->Options.contains(key))
        return default_value;

    return this->Options[key].toString();
}

void Configuration::SetValue(QString key, QVariant value)
{
    if (!this->Options.contains(key))
        this->Options.insert(key, value);

    // Now if the value exists we just update it
    // some autosave might do here
    this->Options[key] = value;
}

void Configuration::Load()
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings settings;
    if (this->configuration_path.isEmpty())
        settings = QSettings(CONFIGURATION_FILE, QSettings::IniFormat);
    else
        settings = QSettings(this->configuration_path, QSettings::IniFormat);
    foreach (QString key, settings.allKeys())
        this->SetValue(key, settings.value(key));
    qDebug() << (QString("Configuration path: ") + settings.fileName());
}

void Configuration::Save()
{
    QSettings settings;
    if (this->configuration_path.isEmpty())
        settings = QSettings(CONFIGURATION_FILE, QSettings::IniFormat);
    else
        settings = QSettings(this->configuration_path, QSettings::IniFormat);
    foreach (QString key, this->Options.keys())
        settings.setValue(key, this->Options[key]);
    settings.sync();
}


