//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "keyfilter.h"
#include "colorbox.h"
#include "inputbox.h"
#include "grumpyconf.h"
#include "scrollbackframe.h"
#include "stextbox.h"
#include <QKeyEvent>
#include <QEvent>

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
        if (keyEvent->modifiers().testFlag(Qt::AltModifier) || keyEvent->modifiers().testFlag(Qt::ShiftModifier))
            return QObject::eventFilter(obj, event);
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier))
        {
            if (keyEvent->key() == Qt::Key_U)
            {
                this->parentInput->InsertAtCurrentPosition(QString((char)1));
                return true;
            }
            if (keyEvent->key() == Qt::Key_B)
            {
                this->parentInput->InsertAtCurrentPosition(QString((char)2));
                return true;
            }
            if (keyEvent->key() == Qt::Key_I)
            {
                this->parentInput->InsertAtCurrentPosition(QString((char)16));
                return true;
            }
            if (keyEvent->key() == Qt::Key_K)
            {
                this->parentInput->InsertAtCurrentPosition(QString((char)3));
                if (!this->parentInput->IsSecure() && CONF->GetColorBoxShow())
                {
                    ColorBox color_box(this->parentInput, this->parentInput);
                    color_box.exec();
                }
                return true;
            }
            return QObject::eventFilter(obj, event);
        }
        if (keyEvent->key() == Qt::Key_Down)
        {
            this->parentInput->History();
            return true;
        }
        if (keyEvent->key() == Qt::Key_Up)
        {
            this->parentInput->History(true);
            return true;
        }
        if (keyEvent->key() == Qt::Key_Tab)
        {
            this->parentInput->Complete();
            return true;
        }
        if (keyEvent->key() == Qt::Key_Return)
        {
            this->parentInput->ProcessInput();
            return true;
        }
        // Forward these to scrollback
        if (keyEvent->key() == Qt::Key_PageUp || keyEvent->key() == Qt::Key_PageDown)
        {
            this->parentInput->GetParent()->GetSTextBox()->SendEvent(keyEvent);
        }
    }
    return QObject::eventFilter(obj, event);
}

