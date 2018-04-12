//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "systemcmds.h"
#include "mainwindow.h"
#include "grumpyconf.h"
#include "scrollbacksmanager.h"
#include "scrollbackframe.h"
#include "corewrapper.h"
#include "uiscript.h"
#include <libirc/libircclient/channel.h>
#include <libirc/libircclient/network.h>
#include <libirc/libirc/serveraddress.h>
#include <libgp/gp.h>
#include <libcore/core.h>
#include <libcore/configuration.h>
#include <libcore/eventhandler.h>
#include <libcore/exception.h>
#include <libcore/generic.h>
#include <libcore/grumpydsession.h>
#include <libcore/ircsession.h>
#include <libcore/scriptextension.h>

using namespace GrumpyIRC;

int SystemCmds::Exit(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    // quit message
    QString quit_msg = command_args.ParameterLine;
    MainWindow::Exit();
    return 0;
}

int SystemCmds::Alias(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    if (!command_args.Parameters.count())
    {
        QHash<QString, QString> hash = CoreWrapper::GrumpyCore->GetCommandProcessor()->GetAliasRdTable();
        unsigned int size = static_cast<unsigned int>(Generic::LongestString(hash.keys()));
        foreach (QString item, hash.keys())
            MainWindow::Main->WriteToCurrentWindow(Generic::ExpandedString(item, size, size) + " -> " + hash[item]);
        return 0;
    }

    if (command_args.Parameters.count() == 1)
    {
        GRUMPY_ERROR("This need to provide 2 or more parameters for this to work");
        return 1;
    }

    if (CoreWrapper::GrumpyCore->GetCommandProcessor()->Exists(command_args.Parameters[0]))
    {
        GRUMPY_ERROR("This name is already used by some other");
        return 1;
    }

    QString value = command_args.ParameterLine.mid(command_args.Parameters[0].size() + 1);
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterAlias(command_args.Parameters[0], value);
    return 0;
}

int SystemCmds::Echo(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    MainWindow::Main->GetCurrentScrollbackFrame()->InsertText(command_args.ParameterLine);
    return 0;
}

int SystemCmds::Notice(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    if (command_args.Parameters.count() < 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires some text"));
        return 1;
    }
    Scrollback *sx = MainWindow::Main->GetScrollbackManager()->GetCurrentScrollback()->GetScrollback();
    if (sx->GetType() == ScrollbackType_System)
    {
        if (command_args.Parameters.count() < 2)
        {
            GRUMPY_ERROR(QObject::tr("This command requires target and some text"));
            return 1;
        }
        QString target = command_args.Parameters[0];
        QString text = command_args.ParameterLine.mid(target.size() + 1);
        Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
        sx->GetSession()->SendNotice(sx, target, text);
    }
    else
    {
        sx->GetSession()->SendNotice(sx, command_args.ParameterLine);
    }
    return 0;
}

int SystemCmds::SendMessage(SystemCommand *command, CommandArgs command_args)
{
    if (command_args.Parameters.size() < 2)
    {
        GRUMPY_ERROR("You need to provide at least 2 parameters");
        return 1;
    }

    // MSG should just echo the message to current window
    QString target = command_args.Parameters[0];
    QString text = command_args.ParameterLine.mid(target.size() + 1);
    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    sx->GetSession()->SendMessage(sx, target, text);
    return 0;
}

int SystemCmds::NextSessionNick(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    if (command_args.Parameters.count() != 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires precisely 1 parameter"));
        return 1;
    }
    GCFG->SetValue("next-session-nick", command_args.ParameterLine);
    return 0;
}

int SystemCmds::Nick(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    // get a current scrollback
    ScrollbackFrame *scrollback = MainWindow::Main->GetScrollbackManager()->GetCurrentScrollback();
    if (!scrollback)
        throw new GrumpyIRC::NullPointerException("scrollback", BOOST_CURRENT_FUNCTION);
    if (command_args.Parameters.count() != 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires precisely 1 parameter"));
        return 1;
    }
    if (scrollback->GetSession())
    {
        scrollback->GetSession()->SendRaw(scrollback->GetScrollback(), "NICK " + command_args.Parameters[0]);
        return 0;
    }
    else
    {
        CONF->SetNick(command_args.Parameters[0]);
        scrollback->InsertText(QString("Your default nick was changed to " + command_args.Parameters[0]));
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
    while (level < 2 && value > 1024)
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
    if (!scrollback->GetSession())
    {
        GRUMPY_ERROR(QObject::tr("You can only use this command in windows that are attached to some network"));
        return 1;
    }
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
    Generic::HostInfo h = Generic::GetHostPortInfo(command_args.Parameters[0], GP_DEFAULT_SSL_PORT);
    if (h.Invalid)
    {
        GRUMPY_ERROR("Invalid host: " + command_args.Parameters[0]);
        return 1;
    }
    MainWindow::Main->OpenGrumpy(h.Host, h.Port, command_args.Parameters[1], command_args.Parameters[2], true);
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
    if (command_args.Parameters.count() < 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires 1 or 2 parameters"));
        return 1;
    }
    QString target = command_args.Parameters.first();
    QString text;
    if (command_args.Parameters.count() > 1)
        text = command_args.ParameterLine.mid(target.size() + 1);
    ScrollbackFrame *scrollback = MainWindow::Main->GetScrollbackManager()->GetCurrentScrollback();
    if (!scrollback->GetSession())
    {
        GRUMPY_ERROR("You can't use query in this window");
        return 2;
    }
    scrollback->GetSession()->Query(scrollback->GetScrollback(), target, text);
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
    Generic::HostInfo hn = Generic::GetHostPortInfo(command_args.Parameters[0], GP_DEFAULT_PORT);
    if (hn.Invalid)
    {
        GRUMPY_ERROR("Invalid host: " + command_args.Parameters[0]);
        return 1;
    }
    MainWindow::Main->OpenGrumpy(hn.Host, hn.Port, command_args.Parameters[1], command_args.Parameters[2], false);
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
        GRUMPY_ERROR("You need to provide some user name to kick, you can also use channel name as optional first parameter, if ommited current channel will be used");
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

int SystemCmds::Uptime(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command_args);
    Q_UNUSED(command);
    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    QDateTime uptime = CONF->GetConfiguration()->GetStartupDateTime();
    int days, hours, minutes, seconds;
    Generic::SecondsToTimeSpan(uptime.secsTo(QDateTime::currentDateTime()), &days, &hours, &minutes, &seconds);
    sx->InsertText("Uptime since " + uptime.toString() + ": " + QString::number(days) + " days " +
                   Generic::DoubleDigit(hours) + ":" + Generic::DoubleDigit(minutes) + ":" + Generic::DoubleDigit(seconds));
    return 0;
}

int SystemCmds::CTCP(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    // if there is no parameter we throw some error
    if (command_args.Parameters.count() < 2)
    {
        GRUMPY_ERROR(QObject::tr("This command requires exactly 2 parameters"));
        return 0;
    }
    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    if (!sx->GetSession())
    {
        GRUMPY_ERROR("You can only use this command in a connected server window");
        return 0;
    }
    QString parameters = "";
    if (command_args.Parameters.count() > 2)
        parameters = command_args.ParameterLine.mid(command_args.Parameters[0].length() + command_args.Parameters[1].length() + 2);
    sx->GetSession()->SendCTCP(sx, command_args.Parameters[0], command_args.Parameters[1].toUpper(), parameters);
    return 0;
}

int SystemCmds::UnAlias(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    if (command_args.Parameters.count() != 1)
    {
        GRUMPY_ERROR("This need to provide 1 parameter for this to work");
        return 1;
    }

    CoreWrapper::GrumpyCore->GetCommandProcessor()->UnregisterAlias(command_args.Parameters[0]);
    return 0;
}

int SystemCmds::Topic(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    ScrollbackFrame *scrollback = MainWindow::Main->GetScrollbackManager()->GetCurrentScrollback();
    if (!scrollback->GetSession() || !scrollback->GetSession()->IsConnected() || scrollback->GetScrollback()->GetType() != ScrollbackType_Channel)
    {
        GRUMPY_ERROR(QObject::tr("You can only use this command in channel windows that are connected to some network"));
        return 1;
    }
    if (command_args.ParameterLine.isEmpty())
    {
        // User wants to retrieve existing topic
        scrollback->GetSession()->SendRaw(scrollback->GetScrollback(), "TOPIC " + scrollback->GetScrollback()->GetTarget());
    } else
    {
        scrollback->GetSession()->SendRaw(scrollback->GetScrollback(), "TOPIC " + scrollback->GetScrollback()->GetTarget() + " :" + command_args.ParameterLine);
    }
    return 0;
}

int SystemCmds::KickBan(SystemCommand *command, CommandArgs command_args)
{
    (void)command;
    if (command_args.ParameterLine.isEmpty())
    {
        GRUMPY_ERROR("You need to provide some user name to kick, you can also use channel name as optional first parameter, if ommited current channel will be used");
        return 1;
    }

    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    if (!sx->GetSession())
    {
        GRUMPY_ERROR("You can only use this command in a connected server window");
        return 2;
    }
    if (!command_args.Parameters[0].startsWith('#') && sx->GetType() != ScrollbackType_Channel)
    {
        GRUMPY_ERROR("This is not a channel window!!");
        return 4;
    }

    SystemCmds::Ban(command, command_args);
    SystemCmds::KICK(command, command_args);
    return 0;
}

int SystemCmds::Ban(SystemCommand *command, CommandArgs command_args)
{
    (void)command;
    if (command_args.ParameterLine.isEmpty())
    {
        GRUMPY_ERROR("You need to provide name of channel (optional) or at least name of user you want to ban");
        return 1;
    }

    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    NetworkSession *session = sx->GetSession();
    if (!session)
    {
        GRUMPY_ERROR("You can only use this command in a connected server window");
        return 1;
    }
    if (!command_args.Parameters[0].startsWith('#') && sx->GetType() != ScrollbackType_Channel)
    {
        GRUMPY_ERROR("This is not a channel window!!");
        return 3;
    }
    QString channel_name, target;
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
        return 4;
    }
    target = command_args.Parameters[0];
    // We need to find this user in channel
    libircclient::Network *network = session->GetNetwork(sx);
    if (!network)
        return 2;
    libircclient::Channel *channel = network->GetChannel(channel_name);
    if (!channel)
    {
        GRUMPY_ERROR("Unknown channel, are you in it?");
        return 5;
    }
    libircclient::User *target_user = channel->GetUser(target);
    if (!target_user)
    {
        GRUMPY_ERROR("No such user in " + channel_name);
        return 6;
    }
    session->SendRaw(sx, "MODE " + channel_name + " +b " + CONF->GetMaskForUser(target_user));
    return 0;
}

int SystemCmds::Script(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    if (command_args.ParameterLine.isEmpty())
    {
        GRUMPY_ERROR("This need to provide file name for this to work");
        return 1;
    }

    UiScript *extension = new UiScript();
    QString error;
    if (!extension->Load(command_args.ParameterLine, &error))
    {
        delete extension;
        GRUMPY_ERROR(error);
        return 1;
    }
    Core::GrumpyCore->RegisterExtension(extension);
    return 0;
}

int SystemCmds::RemoveScript(SystemCommand *command, CommandArgs command_args)
{
    (void)command;
    if (command_args.ParameterLine.isEmpty())
    {
        GRUMPY_ERROR("This need to provide name of script for this to work, use /grumpy.script.list to list them");
        return 1;
    }

    ScriptExtension *e = ScriptExtension::GetExtensionByName(command_args.ParameterLine);
    if (!e)
    {
        GRUMPY_ERROR("No such extension loaded, use /grumpy.script.list to list them");
        return 1;
    }
    e->Unload();
    Core::GrumpyCore->UnregisterExtension(e);
    delete e;
    return 0;
}

int SystemCmds::ScriptList(SystemCommand *command, CommandArgs command_args)
{
    (void)command_args;
    (void)command;
    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    QList<ScriptExtension*> extensions = ScriptExtension::GetExtensions();
    if (extensions.isEmpty())
    {
        sx->InsertText("No scripts loaded");
        return 0;
    }
    sx->InsertText("Name                | Author      | Version   | Description");
    sx->InsertText("--------------------+-------------+-----------+---------------------");
    foreach (ScriptExtension *ext, extensions)
    {
        sx->InsertText(Generic::ExpandedString(ext->GetName(), 20, 20) + "| " + Generic::ExpandedString(ext->GetAuthor(), 12, 12) + "| " +
                       Generic::ExpandedString(ext->GetVersion(), 10, 10) + "| " + ext->GetDescription());
    }
    return 0;
}

static bool ReloadExtension(QString name)
{
    ScriptExtension *e = ScriptExtension::GetExtensionByName(name);
    if (!e)
    {
        GRUMPY_ERROR("No such extension loaded, use /grumpy.script.list to list them");
        return 1;
    }
    QString extension_path = e->GetPath();
    e->Unload();
    Core::GrumpyCore->UnregisterExtension(e);
    delete e;
    UiScript *extension = new UiScript();
    QString error;
    if (!extension->Load(extension_path, &error))
    {
        delete extension;
        GRUMPY_ERROR(error);
        return false;
    }
    Core::GrumpyCore->RegisterExtension(extension);
    return true;
}

int SystemCmds::ScriptReload(SystemCommand *command, CommandArgs command_args)
{
    (void)command;
    if (command_args.ParameterLine.isEmpty())
    {
        GRUMPY_ERROR("This need to provide name of script for this to work, use /grumpy.script.list to list them");
        return 1;
    }

    if (ReloadExtension(command_args.ParameterLine))
        return 0;
    else
        return 1;
}

int SystemCmds::ScriptReloadAll(SystemCommand *command, CommandArgs command_args)
{
    (void)command_args;
    (void)command;
    QStringList el;
    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    QList<ScriptExtension*> extensions = ScriptExtension::GetExtensions();

    if (extensions.isEmpty())
    {
        sx->InsertText("No scripts loaded");
        return 0;
    }

    foreach (ScriptExtension *ext, extensions)
        el.append(ext->GetName());

    foreach (QString name, el)
        ReloadExtension(name);

    return 0;
}
