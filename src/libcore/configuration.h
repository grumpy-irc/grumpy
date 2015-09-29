//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "definitions.h"
#include "libcore_global.h"

#include <QString>
#include <QVariant>
#include <QHash>

namespace GrumpyIRC
{
    class LIBCORESHARED_EXPORT Configuration
    {
        public:
            static QString GetVersion();

            Configuration();
            QVariant GetValue(QString key);
            QString GetValueAsBool(QString key, bool none = false);
            void SetValue(QString key, QVariant value);
            void SetValue(QString key, bool value);
            void SetValue(QString key, int value);
            void SetValue(QString key, QString value);
            void Load();
            void Save();
			unsigned int Verbosity;

        private:
            QHash<QString, QVariant> Options;
    };

    inline void Configuration::SetValue(QString key, bool value) { this->SetValue(key, QVariant(value)); }
    inline void Configuration::SetValue(QString key, int value) { this->SetValue(key, QVariant(value)); }
    inline void Configuration::SetValue(QString key, QString value) { this->SetValue(key, QVariant(value)); }
}

#endif // CONFIGURATION_H
