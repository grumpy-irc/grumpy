//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QKeyEvent>
#include <QEvent>
#include "inputbox.h"
#include "keyfilter.h"

using namespace GrumpyIRC;

KeyFilter::KeyFilter(InputBox *parent) : QObject(parent)
{
    this->parentInput = parent;
}

bool KeyFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return)
        {
            this->parentInput->ProcessInput();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

