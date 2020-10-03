//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2020

#include "grumpydapp.h"
#include "../libcore/eventhandler.h"
#include "../libcore/core.h"
#include "../libcore/exception.h"

using namespace GrumpyIRC;

bool GrumpydApp::notify(QObject *receiver, QEvent *event)
{
    bool done = true;
    try
    {
        done = QCoreApplication::notify(receiver, event);
    } catch (Exception *ex)
    {
        GRUMPY_ERROR("FATAL exception: " + ex->GetMessage() + " at " + ex->GetSource());
        delete ex;
        QCoreApplication::exit(ex->ErrorCode());
    } catch (Exception &ex)
    {
        GRUMPY_ERROR("FATAL exception: " + ex.GetMessage() + " at " + ex.GetSource());
        QCoreApplication::exit(ex.ErrorCode());
    }
    return done;
}
