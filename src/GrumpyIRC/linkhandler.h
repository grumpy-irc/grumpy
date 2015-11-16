//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef LINKHANDLER_H
#define LINKHANDLER_H

#include <QList>
//#include <QString>
#include <QMutex>
#include <QThread>

namespace GrumpyIRC
{
    class LinkHandler_Priv : public QThread
    {
        public:
            LinkHandler_Priv();
            void run();
            static void pub_sleep(unsigned int time);
            bool IsRunning;
            bool IsOffline;
            QMutex mutex;
            QList<QString> links;
    };

    class LinkHandler
    {
        public:
            LinkHandler();
            ~LinkHandler();
            void OpenLink(QString url);

        private:
            LinkHandler_Priv thread;
    };
}

#endif // LINKHANDLER_H
