//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include "linkhandler.h"
#include <QUrl>
#include <QDesktopServices>

using namespace GrumpyIRC;

LinkHandler_Priv::LinkHandler_Priv()
{
    this->IsRunning = true;
    this->IsOffline = false;
}

void LinkHandler::OpenLink(const QString& url)
{
    // Queue for open

    this->thread.mutex.lock();
    this->thread.links.append(url);
    this->thread.mutex.unlock();
}

LinkHandler::LinkHandler()
{
    this->thread.start();
}

LinkHandler::~LinkHandler()
{
    this->thread.IsRunning = false;
    while (!this->thread.IsOffline)
        LinkHandler_Priv::pub_sleep(10);
}

void LinkHandler_Priv::pub_sleep(unsigned int time)
{
    QThread::msleep(time);
}

void LinkHandler_Priv::run()
{
    while (this->IsRunning)
    {
        if (this->links.count())
        {
            QString url;
            this->mutex.lock();
            if (this->links.count())
            {
                url = this->links.first();
                this->links.removeAt(0);
            }
            this->mutex.unlock();
            QDesktopServices::openUrl(QUrl(url));
        }
        LinkHandler_Priv::usleep(200000);
    }
    this->IsOffline = true;
}
