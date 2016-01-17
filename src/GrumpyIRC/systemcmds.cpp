//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "systemcmds.h"
#include "mainwindow.h"
#include "grumpyconf.h"
#include "scrollbacksmanager.h"
#include "scrollbackframe.h"
#include "corewrapper.h"
#include "../libirc/libircclient/channel.h"
#include "../libirc/libircclient/network.h"
#include "../libirc/libirc/serveraddress.h"
#include "../libgp/gp.h"
#include "../libcore/core.h"
#include "../libcore/configuration.h"
#include "../libcore/eventhandler.h"
#include "../libcore/exception.h"
#include "../libcore/generic.h"
#include "../libcore/grumpydsession.h"
#include "../libcore/ircsession.h"

using namespace GrumpyIRC;

int SystemCmds::Exit(SystemCommand *command, CommandArgs args)
{
    Q_UNUSED(command);
    // quit message
    QString quit_msg = args.ParameterLine;
    MainWindow::Exit();
    return 0;
}

int SystemCmds::Alias(SystemCommand *command, CommandArgs args)
{
    Q_UNUSED(command);
    if (!args.Parameters.count())
    {
        QHash<QString, QString> hash = CoreWrapper::GrumpyCore->GetCommandProcessor()->GetAliasRdTable();
        unsigned int size = static_cast<unsigned int>(Generic::LongestString(hash.keys()));
        foreach (QString item, hash.keys())
            MainWindow::Main->WriteToCurrentWindow(Generic::ExpandedString(item, size, size) + " -> " + hash[item]);
        return 0;
    }

    if (args.Parameters.count() == 1)
    {
        GRUMPY_ERROR("This need to provide 2 or more parameters for this to work");
        return 1;
    }

    if (CoreWrapper::GrumpyCore->GetCommandProcessor()->Exists(args.Parameters[0]))
    {
        GRUMPY_ERROR("This name is already used by some other");
        return 1;
    }

    QString value = args.ParameterLine.mid(args.Parameters[0].size() + 1);
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterAlias(args.Parameters[0], value);
    return 0;
}

int SystemCmds::Echo(SystemCommand *command, CommandArgs args)
{
    Q_UNUSED(command);
    MainWindow::Main->GetCurrentScrollbackFrame()->InsertText(args.ParameterLine);
    return 0;
}

int SystemCmds::Notice(SystemCommand *command, CommandArgs args)
{
    Q_UNUSED(command);
    if (args.Parameters.count() < 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires some text"));
        return 1;
    }
    Scrollback *sx = MainWindow::Main->GetScrollbackManager()->GetCurrentScrollback()->GetScrollback();
    if (sx->GetType() == ScrollbackType_System)
    {
        if (args.Parameters.count() < 2)
        {
            GRUMPY_ERROR(QObject::tr("This command requires target and some text"));
            return 1;
        }
        QString target = args.Parameters[0];
        QString text = args.ParameterLine.mid(target.size() + 1);
        Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
        sx->GetSession()->SendNotice(sx, target, text);
    }
    else
    {
        sx->GetSession()->SendNotice(sx, args.ParameterLine);
    }
    return 0;
}

int SystemCmds::SendMessage(SystemCommand *command, CommandArgs args)
{
    if (args.Parameters.size() < 2)
    {
        GRUMPY_ERROR("You need to provide at least 2 parameters");
        return 1;
    }

    // MSG should just echo the message to current window
    QString target = args.Parameters[0];
    QString text = args.ParameterLine.mid(target.size() + 1);
    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    sx->GetSession()->SendMessage(sx, target, text);
    return 0;
}

int SystemCmds::NextSessionNick(SystemCommand *command, CommandArgs args)
{
    Q_UNUSED(command);
    if (args.Parameters.count() != 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires precisely 1 parameter"));
        return 1;
    }
    GCFG->SetValue("next-session-nick", args.ParameterLine);
    return 0;
}

int SystemCmds::Nick(SystemCommand *command, CommandArgs args)
{
    Q_UNUSED(command);
    // get a current scrollback
    ScrollbackFrame *scrollback = MainWindow::Main->GetScrollbackManager()->GetCurrentScrollback();
    if (!scrollback)
        throw new GrumpyIRC::NullPointerException("scrollback", BOOST_CURRENT_FUNCTION);
    if (args.Parameters.count() != 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires precisely 1 parameter"));
        return 1;
    }
    if (scrollback->GetSession())
    {
        scrollback->GetSession()->SendRaw(scrollback->GetScrollback(), "NICK " + args.Parameters[0]);
        return 0;
    }
    else
    {
        CONF->SetNick(args.Parameters[0]);
        scrollback->InsertText(QString("Your default nick was changed to " + args.Parameters[0]));
        return 0;
    }
}

static QString human_read(unsigned long long input)
{
    if (input < 1025)
        return "";
    int level = 0;
    double value = static_cast<double>(input);
    QString units;
    while (level < 2 || value > 1024)
    {
        level++;
        switch (level)
        {
            case 1:
                units = "KB";
                break;
            case 2:
                units = "MB";
                break;
        }
        value = value / 1024;
    }
    value = static_cast<double>(qRound64(value * 100)) / 100;
    return " (" + QString::number(value) + " " + units +  QString(")");
}

int SystemCmds::Netstat(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command_args);
    Q_UNUSED(command);
    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    if (!sx->GetSession() || !Generic::IsGrumpy(sx))
        return 2;
    GrumpydSession *session = (GrumpydSession*)sx->GetSession();
    unsigned long long cr, cs, ur, us;
    cr = session->GetCompressedBytesRcvd();
    cs = session->GetCompressedBytesSent();
    ur = session->GetBytesRcvd();
    us = session->GetBytesSent();
    sx->InsertText("Network stats for this grumpy session:");
    sx->InsertText("-----------------------------------------------------");
    sx->InsertText("Compressed bytes rcvd: " + QString::number(cr) + human_read(cr));
    sx->InsertText("Compressed bytes sent: " + QString::number(cs) + human_read(cs));
    sx->InsertText("Bytes rcvd: " + QString::number(ur) + human_read(ur));
    sx->InsertText("Bytes sent: " + QString::number(us) + human_read(us));
    sx->InsertText("-----------------------------------------------------");
    if (cs > 0 && cr > 0)
    {
        double ratio_rcvd = (((double)cr - (double)ur) / (double)ur) * -100;
        double ratio_sent = (((double)cs - (double)us) / (double)us) * -100;
        sx->InsertText("Compression ratio for rcvd: " + QString::number(ratio_rcvd));
        sx->InsertText("Compression ratio for sent: " + QString::number(ratio_sent));
    }
    sx->InsertText("-----------------------------------------------------");
    sx->InsertText("Packets rcvd: " + QString::number(session->GetPacketsRcvd()));
    sx->InsertText("Packets sent: " + QString::number(session->GetPacketsSent()));
    return 0;
}

int SystemCmds::RAW(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    if (command_args.Parameters.count() < 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires some text"));
        return 1;
    }
    ScrollbackFrame *scrollback = MainWindow::Main->GetScrollbackManager()->GetCurrentScrollback();
    scrollback->GetSession()->SendRaw(scrollback->GetScrollback(), command_args.ParameterLine);
    return 0;
}

int SystemCmds::Grumpy(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    // if there is no parameter we throw some error
    if (command_args.Parameters.count() < 3)
    {
        GRUMPY_ERROR(QObject::tr("This command requires exactly 3 parameters"));
        return 0;
    }
    MainWindow::Main->OpenGrumpy(command_args.Parameters[0], GP_DEFAULT_SSL_PORT, command_args.Parameters[1], command_args.Parameters[2], true);
    return 0;
}

int SystemCmds::Act(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    if (command_args.Parameters.count() < 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires some text"));
        return 1;
    }
    ScrollbackFrame *scrollback = MainWindow::Main->GetScrollbackManager()->GetCurrentScrollback();
    if (!scrollback->GetSession() || !scrollback->GetSession()->IsConnected() || scrollback->GetScrollback()->GetType() == ScrollbackType_System)
    {
        GRUMPY_ERROR(QObject::tr("You can only use this command in channel or user windows of connected networks"));
        return 2;
    }
    scrollback->GetSession()->SendAction(scrollback->GetScrollback(), command_args.ParameterLine);
    return 0;
}

int SystemCmds::GrumpyLink(SystemCommand *command, CommandArgs command_args)
{
    if (command_args.ParameterLine.isEmpty())
    {
        GRUMPY_ERROR("No parameters");
        return 0;
    }

    MainWindow::Main->OpenUrl(command_args.ParameterLine);
    return 0;
}

int SystemCmds::Query(SystemCommand *command, CommandArgs command_args)
{
    (void)command;
    return 0;
}

int SystemCmds::UnsecureGrumpy(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    // if there is no parameter we throw some error
    if (command_args.Parameters.count() < 3)
    {
        GRUMPY_ERROR(QObject::tr("This command requires exactly 3 parameters"));
        return 0;
    }
    MainWindow::Main->OpenGrumpy(command_args.Parameters[0], GP_DEFAULT_PORT, command_args.Parameters[1], command_args.Parameters[2], false);
    return 0;
}

int SystemCmds::Server(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    // if there is no parameter we throw some error
    if (command_args.Parameters.count() < 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires a parameter"));
        return 0;
    }
    // This command is much more tricky than you think
    // we need to first check if we are going to open
    // new server within Grumpy itself, or in grumpyd
    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    if (sx->GetSession() && sx->GetSession()->GetType() == SessionType_Grumpyd)
    {
        // Current window belongs to grumpyd
        GrumpydSession *session = (GrumpydSession*)sx->GetSession();
        // Connect to IRC
        session->Open(libirc::ServerAddress(command_args.Parameters[0]));
        sx->InsertText("Requested connection to IRC server from grumpyd, please wait.");
        return 0;
    }

    // get the server host
    MainWindow::Main->OpenServer(libirc::ServerAddress(command_args.Parameters[0]));
    return 0;
}

int SystemCmds::JOIN(SystemCommand *command, CommandArgs command_args)
{
    (void)command;

    if (command_args.ParameterLine.isEmpty())
    {
        GRUMPY_ERROR("You need to provide some channel names to join, they can be even separated by comma or space");
        return 0;
    }

    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    if (!sx->GetSession())
    {
        GRUMPY_ERROR("You can only use this command in a connected server window");
        return 0;
    }

    QList<QString> channel_list_t, channel_list_d;
    if (command_args.ParameterLine.contains(" "))
        channel_list_t = command_args.ParameterLine.split(' ');
    if (command_args.ParameterLine.contains(","))
        channel_list_t = command_args.ParameterLine.split(',');

    if (channel_list_t.isEmpty())
    {
        // Requested join for 1 item only
        sx->GetSession()->SendRaw(sx, "JOIN " + command_args.ParameterLine);
        // There is nothing else to do
        return 0;
    }

    foreach (QString c, channel_list_t)
    {
        c = c.replace(" ", "").replace(",", "");
        channel_list_d.append(c);
    }

    foreach (QString channel, channel_list_d)
        sx->GetSession()->SendRaw(sx, "JOIN " + channel);
    return 0;
}

int SystemCmds::KICK(SystemCommand *command, CommandArgs command_args)
{
    (void)command;
    if (command_args.ParameterLine.isEmpty())
    {
        GRUMPY_ERROR("You need to provide some channel names to join, they can be even separated by comma or space");
        return 0;
    }

    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    if (!sx->GetSession())
    {
        GRUMPY_ERROR("You can only use this command in a connected server window");
        return 0;
    }
    if (!command_args.Parameters[0].startsWith('#') && sx->GetType() != ScrollbackType_Channel)
    {
        GRUMPY_ERROR("This is not a channel window!!");
        return 0;
    }
    QString channel_name;
    if (command_args.Parameters[0].startsWith('#'))
    {
        channel_name = command_args.Parameters[0];
        command_args.Parameters.removeAt(0);
    } else
    {
        channel_name = sx->GetTarget();
    }
    if (command_args.Parameters.size() == 0)
    {
        GRUMPY_ERROR("No target");
        return 0;
    }
    QString target, reason;
    if (command_args.Parameters.size() == 1)
    {
        target = command_args.Parameters[0];
        reason = CONF->GetDefaultKickReason();
    }
    if (command_args.Parameters.size() > 1)
    {
        target = command_args.Parameters[0];
        reason = command_args.ParameterLine.mid(command_args.ParameterLine.indexOf(target) + target.size() + 1);
    }
    sx->GetSession()->SendRaw(sx, "KICK " + channel_name + " " + target + " :" + reason);
    return 0;
}
