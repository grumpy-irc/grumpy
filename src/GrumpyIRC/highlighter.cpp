//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "highlighter.h"
#include "grumpyconf.h"

using namespace GrumpyIRC;

QList<Highlighter*> Highlighter::Highlighter_Data;

void Highlighter::Init()
{
    if (!CONF->FirstRun())
        return;
    new Highlighter("$nick");
}

bool Highlighter::IsMatch(ScrollbackItem *text)
{
    foreach (Highlighter *hx, Highlighter_Data)
    {
        if (hx->IsMatching(text))
            return true;
    }
    return false;
}

Highlighter::Highlighter(QString text)
{
    this->CaseSensitive = false;
    this->IsRegex = false;
    this->definition = text;
    Highlighter_Data.append(this);
}

Highlighter::~Highlighter()
{
    Highlighter_Data.removeOne(this);
}

bool Highlighter::IsMatching(ScrollbackItem *text)
{

    return false;
}

