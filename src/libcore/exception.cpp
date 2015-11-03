//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "exception.h"

GrumpyIRC::Exception::Exception()
{

}

GrumpyIRC::Exception::Exception(QString Message, QString function_id)
{
    this->message = Message;
    this->source = function_id;
}

QString GrumpyIRC::Exception::GetMessage()
{
    this->source = "(not provided)";
    return this->message;
}

GrumpyIRC::NullPointerException::NullPointerException(QString pointer, QString signature)
{

}
