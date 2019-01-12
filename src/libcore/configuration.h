//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

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
            static void GetVersion(int *major, int *minor, int *revision);

            Configuration();
            virtual ~Configuration();

            // General config
            virtual bool Contains(const QString &key);
            virtual QVariant GetValue(const QString &key);
            virtual void SetAlternativeConfigFile(const QString &file);
            virtual bool GetValueAsBool(const QString &key, bool none = false);
            virtual unsigned int GetValueAsUInt(const QString &key, unsigned int none = 0);
            virtual QString GetValueAsString(const QString &key, const QString &default_value = "");
            virtual int GetValueAsInt(const QString &key, int none = 0);
            virtual float GetValueAsFloat(const QString &key, float none = 0);
            virtual void RemoveValue(const QString &key);
            virtual void SetValue(const QString &key, const QVariant &value);
            virtual void SetValue(QString key, bool value);
            virtual void SetValue(QString key, int value);
            virtual void SetValue(QString key, QString value);
            virtual void SetHomePath(const QString &path);

            // Extension config
            virtual bool        Extension_Contains(const QString &extension, const QString &key);
            virtual QVariant    Extension_GetValue(const QString &extension, const QString &key);
            virtual bool        Extension_GetValueAsBool(const QString &extension, const QString &key, bool none = false);
            virtual QString     Extension_GetValueAsString(const QString &extension, const QString &key, const QString &default_value = "");
            virtual int         Extension_GetValueAsInt(const QString &extension, const QString &key, int none = 0);
            virtual float       Extension_GetValueAsFloat(const QString &extension, const QString &key, float none = 0);
            virtual void        Extension_RemoveValue(const QString &extension, const QString &key);
            virtual void        Extension_SetValue(const QString &extension, const QString &key, const QVariant &value);
            virtual void        Extension_SetValue(const QString &extension, const QString &key, bool value);
            virtual void        Extension_SetValue(const QString &extension, const QString &key, int value);
            virtual void        Extension_SetValue(const QString &extension, const QString &key, const QString &value);

            // Scripting config
            virtual void SetUnsafeScriptFc(bool enabled);
            virtual bool GetUnsafeScriptFc();

            // Miscelancelous
            virtual QString GetHomePath();
            //! System script path - system wide scripts
            virtual QString GetScriptPath();
            //! System extension path - system wide extensions
            virtual QString GetExtensionPath();
            //! User script path
            virtual QString GetHomeScriptPath();
            //! User extension path
            virtual QString GetHomeExtensionPath();
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
