//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "../libcore/scrollback.h"
#include "../libirc/libircclient/network.h"
#include "exception.h"
#include "highlighter.h"
#include "profiler.h"

using namespace GrumpyIRC;

QList<Highlighter*> Highlighter::Highlighter_Data;

/*void Highlighter::Init()
{
    if (!CONF->FirstRun())
        return;
    new Highlighter("$nick");
}*/

bool Highlighter::IsMatch(ScrollbackItem *text, libircclient::Network *network)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    foreach (Highlighter *hx, Highlighter_Data)
    {
        if (hx->IsMatching(text, network))
            return true;
    }
    return false;
}

Highlighter::Highlighter(const QHash<QString, QVariant> &hash)
{
    this->CaseSensitive = false;
    this->IsRegex = false;
    this->Messages = true;
    this->MatchingSelf = false;
    this->Enabled = true;
    Highlighter_Data.append(this);
    this->LoadHash(hash);
}

Highlighter::Highlighter(const QString &text)
{
    this->CaseSensitive = false;
    this->IsRegex = false;
    this->Enabled = true;
    this->definition = text;
    this->Messages = true;
    this->MatchingSelf = false;
    Highlighter_Data.append(this);
}

Highlighter::~Highlighter()
{
    Highlighter_Data.removeOne(this);
}

QHash<QString, QVariant> Highlighter::ToHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(CaseSensitive);
    SERIALIZE(IsRegex);
    SERIALIZE(definition);
    SERIALIZE(Enabled);
    SERIALIZE(Messages);
    SERIALIZE(MatchingSelf);
    return hash;
}

void Highlighter::LoadHash(const QHash<QString, QVariant> &hash)
{
    UNSERIALIZE_BOOL(CaseSensitive);
    UNSERIALIZE_BOOL(IsRegex);
    UNSERIALIZE_BOOL(Enabled);
    UNSERIALIZE_BOOL(Messages);
    UNSERIALIZE_BOOL(MatchingSelf);
    UNSERIALIZE_STRING(definition);
}

bool Highlighter::IsMatching(ScrollbackItem *text, libircclient::Network *network)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (!this->Enabled)
        return false;

    if (text->IsSelf() && !this->MatchingSelf)
        return false;

    if (this->Messages && !(text->GetType() == ScrollbackItemType_Notice || text->GetType() == ScrollbackItemType_Message))
        return false;

    if (this->IsRegex)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        this->regex.setPattern(this->definition);
        if (!this->CaseSensitive)
            this->regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        return this->regex.match(text->GetText()).hasMatch();
#else
        this->regex.setPattern(this->definition);
        if (!this->CaseSensitive)
            this->regex.setCaseSensitivity(Qt::CaseInsensitive);
        return this->regex.indexIn(text->GetText()) != -1;
#endif
    } else
    {
        QString string = this->definition;
        if (network)
            string.replace("$nick", network->GetNick());
        if (!this->CaseSensitive)
        {
            string = string.toLower();
            if (text->GetText().toLower().contains(string))
                return true;
        } else
        {
            if (text->GetText().contains(string))
                return true;
        }
    }

    return false;
}


void GrumpyIRC::Highlighter::SetDefinition(const QString &value)
{
    this->definition = value;
}

void Highlighter::MakeRegex()
{
    this->IsRegex = true;
}

QString Highlighter::GetDefinition() const
{
    return this->definition;
}
