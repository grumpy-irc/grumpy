//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QDebug>
#include "eventhandler.h"

GrumpyIRC::EventHandler::EventHandler()
{

}

GrumpyIRC::EventHandler::~EventHandler()
{

}

void GrumpyIRC::EventHandler::OnMessage(scrollback_id_t ScrollbackID)
{
    //qDebug() << "Message: " +
}

void GrumpyIRC::EventHandler::OnDebug(QString text, unsigned int verbosity)
{
    qDebug() << "DEBUG: " + text;
}

void GrumpyIRC::EventHandler::OnError(QString text)
{
    qDebug() << "ERROR: " + text;
}

void GrumpyIRC::EventHandler::OnSystemLog(QString text)
{
    qDebug() << "INFO:  " + text;
}

