//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "mainwindow.h"
#include "corewrapper.h"
#include "grumpyeventhandler.h"
#include "../libcore/core.h"
#include <QApplication>

using namespace GrumpyIRC;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialize core first
    CoreWrapper::GrumpyCore = new Core();
	CoreWrapper::GrumpyCore->InitCfg();
	CoreWrapper::GrumpyCore->SetSystemEventHandler(new GrumpyEventHandler());

    MainWindow w;
    w.show();

    int ReturnCode = a.exec();
    delete CoreWrapper::GrumpyCore;
    return ReturnCode;
}
