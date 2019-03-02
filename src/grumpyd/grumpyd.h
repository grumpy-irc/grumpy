//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef GRUMPYD_H
#define GRUMPYD_H

#include <QString>
#include "../libcore/exception.h"
#include "listener.h"
#include <QObject>
#include <QTimer>

namespace GrumpyIRC
{
    class DatabaseBackend;

    class Grumpyd : public QObject
    {
            Q_OBJECT
        public:
            static QString GetCFPath();
            static QString GetDFPath();
            static QString GetPathSSLCert();
            static QString GetPathSSLKey();
            static DatabaseBackend *GetBackend();
            static bool SSLIsAvailable();
            static Grumpyd *grumpyd;

            Grumpyd();
            ~Grumpyd();
            void Kill();

        public slots:
            void Main();

        private:
            void initScripts();
            DatabaseBackend *databaseBackend;
            Listener *listener;
            Listener *listenerSSL;
            bool running;

    };
}

#endif // GRUMPYD_H
