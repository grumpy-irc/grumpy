//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef GRUMPYCONF_H
#define GRUMPYCONF_H

#ifdef GCFG
    #undef GCFG
#endif

#include "../libcore/definitions.h"
#include "../libgp/gp.h"
#include <QVariant>

#ifdef CONF
    #undef CONF
#endif
#define GCFG CoreWrapper::GrumpyCore->GetConfiguration()
#define CONF GrumpyIRC::GrumpyConf::Conf

namespace GrumpyIRC
{
    class Configuration;
    class GrumpyConf
    {
        public:
            static GrumpyConf *Conf;
            GrumpyConf();
            Configuration *GetConfiguration();
            QString GetStorage();
            void SetStorage(QString name);
            QString GetQuitMessage();
            void SetNick(QString nick);
            unsigned int GetMaxLoadSize();
            QString GetNick();
            unsigned int GetMaxScrollbackSize();

            QString etc = "etc";
            QString var = "var";
            bool Upgrade = false;
            bool DBMaint = false;
            bool DBTrim = false;
            bool Init;
            bool Stdout = false;
            bool AutoFix;
            int DefaultPort = GP_DEFAULT_PORT;
            int SecuredPort = GP_DEFAULT_SSL_PORT;
            QString PID;
            bool Daemon;
            bool StorageDummy;
            //! This is maximum allowed size of personal storage per user (for BLOB storage, not scrollback storage)
            unsigned long long MaxPersonalStorageSize = 1048576;
    };
}

#endif // GRUMPYCONF_H
