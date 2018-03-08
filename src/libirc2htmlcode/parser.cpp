//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "parser.h"

using namespace irc2htmlcode;

Parser::Parser()
{
    this->TextColor = "#FFFFFF";
    this->TimeColor = "#DCFCAC";
    this->UserColor = "#B4D6FA";
    this->ChannelSymbol = '#';
    this->resolveCacheSize = 2000;
    this->cacheHits = 0;
    this->SeparateOnDotSpace = true;
    this->LinkColor = "#94D7F2";
    this->cacheMiss = 0;
    this->Protocols << "http" << "https" << "ftp" << "irc" << "ircs";
    this->LinkSeparators << ' ' << ',' << '(' << ')';
    this->Separators << ' ' << '.' << ',' << ':' << ';' << '!' << '(' << ')';
    this->SeparatorsPriv << ' ' << '(';
    // We expect a dark background so black and white mixed up
    this->TextColors.insert(0,    "#000000"); // White
    this->TextColors.insert(1,    "#FFFFFF"); // Black
    this->TextColors.insert(2,    "#97ADF7"); // Blue
    this->TextColors.insert(3,    "#A1F797"); // Green
    this->TextColors.insert(4,    "#FFC4CE"); // Light red
    this->TextColors.insert(5,    "#CCB472"); // Brown
    this->TextColors.insert(6,    "#F03CD5"); // Purple
    this->TextColors.insert(7,    "#F0B754"); // Orange
    this->TextColors.insert(8,    "#FCFA6A"); // Yellow
    this->TextColors.insert(9,    "#C3FAC5"); // Light green
    this->TextColors.insert(10,   "#C8FAF7"); // Cyan
    this->TextColors.insert(11,   "#DEFAF8"); // Light cyan
    this->TextColors.insert(12,   "#D2D3FA"); // Light blue
    this->TextColors.insert(13,   "#F7C3F1"); // Pink
    this->TextColors.insert(14,   "#C4C4C4"); // Grey
    this->TextColors.insert(15,   "#DADADA"); // Light grey
}

Parser::~Parser()
{

}

FormattedItem Parser::Process(QString format, QDateTime time, QString user, QString text, QString override_default_text_color)
{
    /*if (this->resolveCache.contains(input))
    {
        this->cacheHits++;
        return this->resolveCache[input];
    }
    this->cacheMiss++;*/
    QString text_color = this->TextColor;
    if (!override_default_text_color.isEmpty())
        text_color = override_default_text_color;
    FormattedItem item;
    item.time = time;
    item.text = text;
    item.user = user;

    item.source = this->EncodeHtml(format);
    item.source.replace("$time", this->formatTime(time));
    item.source.replace("$nick", this->formatUser(user));
    item.source.replace("$string", this->formatText(text, text_color));

    //item.source = input;
    /*
    // Make some space in cache
    while (this->resolveCache.size() > this->resolveCacheSize)
    {
        // Now we remove the first item, but later we need to figure out these with last access time
        this->resolveCache.remove(this->resolveCache.keys().at(0));
    }

    this->resolveCache.insert(input, item);
    */

    return item;
}

QString Parser::EncodeHtml(QString text)
{
    text.replace("<", "&lt;");
    text.replace(">", "&gt;");
    text.replace("  ", "&nbsp;&nbsp;");

    return text;
}

QString Parser::GetStyle()
{
    return "a {\n"\
           "    color: " + this->LinkColor + ";\n"\
                                             "}";
}

QString Parser::linkUrl(QString source, QString protocol)
{
    int position = 0;
    protocol = protocol + "://";
    while (position < source.size() && source.mid(position).contains(protocol))
    {
        position = source.indexOf(protocol, position);
        // find the end of link
        int end = position;
        while (++end < source.size())
        {
            char current_symbol = source[end].toLatin1();
            if (this->SeparateOnDotSpace && current_symbol == '.')
            {
                // If dot is last symbol in text
                if (end + 1 == source.size())
                    break;
                // If it's not but next symbol is a space
                if (end + 1 < source.size() && source[end+1].toLatin1() == ' ')
                    break;
            }
            if (this->LinkSeparators.contains(current_symbol))
                break;
        }
        QString lx = source.mid(position, end - position);
        QString linked_lx = "<a style=\"color:" + this->LinkColor + ";\" href=\"" + lx + "\">" + lx + "</a>";
        source.remove(position, lx.size());
        source.insert(position, linked_lx);
        position += linked_lx.size();
    }
    return source;
}

QString Parser::linkUrls(QString source)
{
    foreach (QString protocol, this->Protocols)
        source = this->linkUrl(source, protocol);
    return source;
}

QString Parser::linkChannels(QString source)
{
    int current_pos = 0;
    int word_start = 0;
    while (current_pos < source.size())
    {
        char current = source[current_pos].toLatin1();
        if (current == ChannelSymbol)
        {
            // This is likely a channel, but only if the preceeding symbol was a separator OR this is a first symbol
            if (current_pos != 0)
            {
                char previous_char = source[current_pos - 1].toLatin1();
                if (!this->SeparatorsPriv.contains(previous_char))
                {
                    current_pos++;
                    continue;
                }
            }
            int channel_end = current_pos;
            while (++channel_end < source.size())
            {
                if (this->Separators.contains(source[channel_end].toLatin1()))
                {
                    break;
                }
            }
            QString channel_name = source.mid(word_start, channel_end - word_start);
            QString channel_link = "<a style=\"color:" + this->LinkColor + ";\" href=\"irc_join://" + channel_name + "\">" + channel_name + "</a>";
            // Now we need to cut the original channel from source
            source.remove(word_start, channel_end - word_start);
            source.insert(word_start, channel_link);
            current_pos = word_start + channel_link.size();
            continue;
        } else if (this->Separators.contains(current))
        {
            word_start = current_pos+1;
        }
        current_pos++;
    }

    return source;
}

static bool isNumber(QChar input)
{
    return input.toLatin1() >= '0' && input.toLatin1() <= '9';
}

QString Parser::replaceSpecials(QString source)
{
    char underline = 1;
    char bold = 2;
    char italic = 16;
    char color = 3;
    char terminated = 15;
    int current_pos = 0;
    bool close_b = false;
    bool close_i = false;
    bool close_u = false;
    bool close_k = false;
    while (current_pos < source.size())
    {
        char current_symbol = source[current_pos].toLatin1();
        if (current_symbol == terminated)
        {
            source = source.remove(current_pos, 1);
            if (close_b)
                source.insert(current_pos, "</b>");
            if (close_u)
                source.insert(current_pos, "</u>");
            if (close_k)
                source.insert(current_pos, "</font>");
            if (close_i)
                source.insert(current_pos, "</i>");
            close_b = false;
            close_u = false;
            close_k = false;
            close_i = false;
        } else if (current_symbol == bold)
        {
            // replace
            source = source.remove(current_pos, 1);
            if (close_b)
                source.insert(current_pos, "</b>");
            else
                source.insert(current_pos, "<b>");
            close_b = !close_b;
        } else if (current_symbol == underline)
        {
            source = source.remove(current_pos, 1);
            if (close_u)
                source.insert(current_pos, "</u>");
            else
                source.insert(current_pos, "<u>");
            close_u = !close_u;
        } else if (current_symbol == color)
        {
            // check if next symbol is a color number
            int color_code = -10;
            if (source.size() <= current_pos + 1)
            {
                // There is nothing else to colorize, if this is closing, we close it, otherwise ignore
                if (close_k)
                {
                    source = source.remove(current_pos, 1);
                    close_k = !close_k;
                    source.insert(current_pos, "</font>");
                }
            } else if (source.size() > current_pos + 2)
            {
                // Check if next 2 are numbers
                QString color_scode;
                // How many characters we need to remove from original string,
                // this varies on size of color code string
                int replace_len = 1;
                if (isNumber(source[current_pos + 1]))
                {
                    replace_len++;
                    color_scode += source[current_pos + 1];
                }
                if (isNumber(source[current_pos + 2]))
                {
                    replace_len++;
                    color_scode += source[current_pos + 2];
                }
                if (!color_scode.isEmpty())
                {
                    color_code = color_scode.toInt();
                    QString color = "";
                    if (color_code < 0 || color_code > 15)
                    {
                        // broken color code
                        // emit some warning?
                        color = "green";
                    } else
                    {
                        color = this->TextColors[color_code];
                    }
                    source = source.remove(current_pos, replace_len);
                    QString result_string = "<font color=\"" + color + "\">";
                    if (close_k)
                    {
                        // someone wants to change the color but the previous color was not closed
                        result_string = "</font>" + result_string;
                    }
                    close_k = true;
                    source.insert(current_pos, result_string);
                } else if (close_k)
                {
                    source = source.remove(current_pos, 1);
                    close_k = !close_k;
                    source.insert(current_pos, "</font>");
                }
            }

        } else if (current_symbol == italic)
        {
            source = source.remove(current_pos, 1);
            if (close_i)
                source.insert(current_pos, "</i>");
            else
                source.insert(current_pos, "<i>");
            close_i = !close_i;
        }
        current_pos++;
    }
    if (close_k)
        source += "</font>";
    if (close_b)
        source += "</b>";
    if (close_u)
        source += "</u>";
    if (close_i)
        source += "</i>";
    return source;
}

QString Parser::formatTime(QDateTime time)
{
    return "<font color=\"" + this->TimeColor + "\">" + this->EncodeHtml(time.toString()) + "</font>";
}

QString Parser::formatUser(QString user)
{
    return "<font color=\"" + this->UserColor + "\">" + this->EncodeHtml(user) + "</font>";
}

QString Parser::formatText(QString text, QString color)
{
    return "<font color=\"" + color + "\">" + this->replaceSpecials(this->linkChannels(this->linkUrls(this->EncodeHtml(text)))) + "</font>";
}
