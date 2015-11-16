//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "linkhandler.h"
#include <QUrl>
#include <QDesktopServices>

using namespace GrumpyIRC;

void LinkHandler::OpenLink(QString url)
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
    this->thread.exit();
}

void LinkHandler_Priv::run()
{
    while (this->isRunning())
    {
        if (this->links.count())
        {
            this->mutex.lock();
            if (this->links.count())
            {
                QDesktopServices::openUrl(QUrl(this->links.first()));
                this->links.removeAt(0);
            }
            this->mutex.unlock();
        }
        this->usleep(200000);
    }
}
