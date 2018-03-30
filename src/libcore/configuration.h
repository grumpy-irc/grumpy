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
#include <QDateTime>
#include <QHash>

namespace GrumpyIRC
{
    class LIBCORESHARED_EXPORT Configuration
    {
        public:
            static QString GetVersion();

            Configuration();
            virtual ~Configuration();

            // General config
            virtual bool Contains(QString key);
            virtual QVariant GetValue(QString key);
            virtual void SetAlternativeConfigFile(QString file);
            virtual bool GetValueAsBool(QString key, bool none = false);
            virtual unsigned int GetValueAsUInt(QString key, unsigned int none = 0);
            virtual QString GetValueAsString(QString key, QString default_value = "");
            virtual int GetValueAsInt(QString key, int none = 0);
            virtual float GetValueAsFloat(QString key, float none = 0);
            virtual void RemoveValue(QString key);
            virtual void SetValue(QString key, QVariant value);
            virtual void SetValue(QString key, bool value);
            virtual void SetValue(QString key, int value);
            virtual void SetValue(QString key, QString value);
            virtual void SetHomePath(QString path);

            // Extension config
            virtual bool        Extension_Contains(QString extension, QString key);
            virtual QVariant    Extension_GetValue(QString extension, QString key);
            virtual bool        Extension_GetValueAsBool(QString extension, QString key, bool none = false);
            virtual QString     Extension_GetValueAsString(QString extension, QString key, QString default_value = "");
            virtual int         Extension_GetValueAsInt(QString extension, QString key, int none = 0);
            virtual float       Extension_GetValueAsFloat(QString extension, QString key, float none = 0);
            virtual void        Extension_RemoveValue(QString extension, QString key);
            virtual void        Extension_SetValue(QString extension, QString key, QVariant value);
            virtual void        Extension_SetValue(QString extension, QString key, bool value);
            virtual void        Extension_SetValue(QString extension, QString key, int value);
            virtual void        Extension_SetValue(QString extension, QString key, QString value);

            // Miscelancelous
            virtual QString GetHomePath();
            virtual QDateTime GetStartupDateTime();
            virtual void Load();
            virtual void Save();
			unsigned int Verbosity;

        protected:
            QString configuration_path;
            QString home_path;
            QHash<QString, QVariant> Options;
            QDateTime startupDateTime;
        private:
            inline QString mkExt(QString e, QString k) { return "ext_" + e + "/" + k; }
    };

    inline void Configuration::SetValue(QString key, bool value) { this->SetValue(key, QVariant(value)); }
    inline void Configuration::SetValue(QString key, int value) { this->SetValue(key, QVariant(value)); }
    inline void Configuration::SetValue(QString key, QString value) { this->SetValue(key, QVariant(value)); }
}

#endif // CONFIGURATION_H
