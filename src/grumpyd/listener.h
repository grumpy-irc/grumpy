//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef LISTENER_H
#define LISTENER_H

#include <QString>
#include <QTcpServer>
#include <QObject>

namespace GrumpyIRC
{
    class Listener : public QTcpServer
    {
        public:
            Listener();
        protected:
            void incomingConnection(qintptr socketDescriptor) Q_DECL_OVERRIDE;
    };
}

#endif // LISTENER_H
