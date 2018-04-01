//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include <QFile>
#include "resources.h"

using namespace GrumpyIRC;

bool Resources::isInit = false;

QString Resources::GetSource(QString path)
{
    //Resources::init();
    QFile file(":" + path);
    if (!file.open(QIODevice::ReadOnly))
        return QString();
    QString result = file.readAll();
    file.close();
    return result;
}

void Resources::init()
{
    if (Resources::isInit)
        return;

    //Q_INIT_RESOURCE(embedded);

    Resources::isInit = true;
}
