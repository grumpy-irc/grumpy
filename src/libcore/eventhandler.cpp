//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include <QDebug>
#include "eventhandler.h"

void GrumpyIRC::EventHandler::OnMessage(scrollback_id_t ScrollbackID)
{
    //qDebug() << "Message: " +
}

void GrumpyIRC::EventHandler::OnDebug(const QString &text, unsigned int verbosity)
{
    qDebug() << "DEBUG: " + text;
}

void GrumpyIRC::EventHandler::OnError(const QString &text)
{
    qDebug() << "ERROR: " + text;
}

void GrumpyIRC::EventHandler::OnSystemLog(const QString &text)
{
    qDebug() << "INFO:  " + text;
}

void GrumpyIRC::EventHandler::OnGrumpydCtorCall(GrumpyIRC::GrumpydSession *session)
{

}

void GrumpyIRC::EventHandler::OnGrumpydDtorCall(GrumpyIRC::GrumpydSession *session)
{

}

