//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "autocompletionengine.h"
#include "core.h"
#include "commandprocessor.h"

using namespace GrumpyIRC;

AutocompletionEngine::AutocompletionEngine()
{
    this->channelPrefix = '#';
    this->WordSeparators << ' ';
}

AutocompletionEngine::~AutocompletionEngine()
{

}

AutocompletionInformation AutocompletionEngine::Execute(AutocompletionInformation input, QList<QString> extra_commands, QList<QString> users, QList<QString> channels)
{
    // This is where the isolated word starts
    //   this pos: v
    // sample text /bla|
    //             ^
    int start;
    AutocompletionInformation results;
    // Isolate the current word which we have cursor at
    QString word = this->getIsolatedWord(input, &start);
    bool successful;
    // First of all we check if it's a command
    if (start == 0 && !word.isEmpty() && word.startsWith(Core::GrumpyCore->GetCommandProcessor()->CommandPrefix))
    {
        QString unprefixed_command = word.mid(1);
        // get a list of all grumpy commands, but suffixed with a space
        // we append space to every word because we want final autocompleted word to be suffixed with it
        // this will also append space to full command name if user hit enter while having cursor on word, so
        // "/server|" is expanded to "/server |" (pipe stands for cursor)
        QList<QString> commands;
        // Fetch aliases here
        foreach(QString alias, Core::GrumpyCore->GetCommandProcessor()->GetAList())
            commands << alias + " ";
        foreach (QString cm, Core::GrumpyCore->GetCommandProcessor()->GetCommands())
            commands << cm + " ";
        foreach (QString cm, extra_commands)
        {
            QString command = cm + " ";
            if (!commands.contains(command))
                commands.append(command);
        }
        results = this->processList(commands, &successful, true, unprefixed_command, start+1, input.FullText);
        if (successful)
            return results;
    }
    // Now let's try channels
    if (!word.isEmpty() && word.startsWith(this->channelPrefix))
    {
        results = this->processList(channels, &successful, true, word, start, input.FullText);
        if (successful)
            return results;
    }
    // Users
    if (!word.isEmpty())
    {
        QList<QString> ulist;
        bool is_start = true;
        int position = input.Position;
        while (position > 0)
        {
            if (input.FullText[position--] == ' ')
            {
                is_start = false;
                break;
            }
        }
        // if we are on start of the sentence, we want to put colon to nickname, otherwise just complete it
        if (is_start)
        {
            foreach (QString ux, users)
                ulist << ux + ": ";
        } else
        {
            ulist = users;
        }
        results = this->processList(ulist, &successful, true, word, start, input.FullText);
        if (successful)
            return results;
    }

    return AutocompletionInformation();
}

QString AutocompletionEngine::replaceWord(QString source, int start, QString sw, QString target)
{
    // we need to cut a middle of string, so bad Qt doesn't seem to have a function for this
    QString result = source.mid(0, start);
    result += source.mid(start + sw.size());
    result.insert(start, target);
    return result;
}

QString AutocompletionEngine::getIsolatedWord(AutocompletionInformation input, int *start_pos)
{
    QString source = input.FullText;
    int start = input.Position;
    // we need to shift start 1 position back, but only if we can
    if (start > 0)
        start--;
    int end = input.Position;
    // first go backwards
    while (start > 0)
    {
        char symbol = source[start-1].toLatin1();
        if (this->WordSeparators.contains(symbol))
        {
            // we reached the start of currently selected word
            break;
        }
        start--;
    }
    while (end < source.size())
    {
        char symbol = source[end+1].toLatin1();
        if (this->WordSeparators.contains(symbol))
        {
            // we reached the EOW of currently selected word
            break;
        }
        end++;
    }
    // let's hope this calculation makes sense
    int length = end - start;
    *start_pos = input.Position - length;
    return source.mid(start, length);
}

QString AutocompletionEngine::getSimilar(QList<QString> words, QString hint)
{
    if (words.isEmpty())
        return hint;
    QString result = hint;
    while (true)
    {
        if (words[0].size() < result.size() + 1)
            return result;
        QString suggested = result + words[0].at(result.size());
        foreach (QString wx, words)
        {
            if (suggested.size() > wx.size())
                return result;
            if (!wx.startsWith(suggested))
                return result;
        }
        result = suggested;
    }
}

AutocompletionInformation AutocompletionEngine::processList(QList<QString> list_of_words_cmp, bool *success, bool case_sensitive, QString word, int start, QString full_text)
{
    *success = true;
    AutocompletionInformation results;
    results.Success = true;
    QStringList potential_candidates;
    if (!case_sensitive)
        word = word.toLower();
    foreach (QString command, list_of_words_cmp)
    {
        QString original_command = command;
        if (!case_sensitive)
            command = command.toLower();
        if (command.startsWith(word))
            potential_candidates.append(original_command);
    }
    if (potential_candidates.count() > 1)
    {
        // there is many candidates, we need to return all of them and expand to suggested part if possible
        foreach (QString suggestion, potential_candidates)
            results.Suggestions.append(suggestion.trimmed());
        QString part = this->getSimilar(potential_candidates);
        results.FullText = this->replaceWord(full_text, start, word, part);
        results.Position = start + part.size();
        return results;
    } else if (potential_candidates.count() == 1)
    {
        // we got an exact match that we need to autocomplete to
        results.FullText = this->replaceWord(full_text, start, word, potential_candidates.at(0));
        results.Position = start + potential_candidates[0].size();
        return results;
    }
    *success = false;
    return results;
}

AutocompletionInformation::AutocompletionInformation()
{
    this->Position = 0;
    this->Success = false;
}
