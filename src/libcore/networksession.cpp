//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "networksession.h"

using namespace GrumpyIRC;

NetworkSession::NetworkSession()
{
    this->createdOn = QDateTime::currentDateTime();
    this->connectedOn = QDateTime::currentDateTime();
    this->isAway = false;
}

NetworkSession::~NetworkSession()
{
    emit this->Event_Deleted();
}

void NetworkSession::SetAway(QString reason)
{
    (void)reason;
    this->isAway = true;
}

void NetworkSession::UnsetAway()
{
    this->isAway = false;
}

bool NetworkSession::IsAway()
{
    return this->isAway;
}

QDateTime NetworkSession::GetCreationDateTime()
{
    return this->createdOn;
}

QDateTime NetworkSession::GetConnectionDateTime()
{
    return this->connectedOn;
}

