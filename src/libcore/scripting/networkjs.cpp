//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "scriptextension.h"
#include "networkjs.h"
#include "../core.h"
#include "../configuration.h"
#include "../commandprocessor.h"
#include "../definitions.h"
#include "../exception.h"
#include "../eventhandler.h"
#include "../networksession.h"
#include "../../libirc/libircclient/network.h"
#include <QHash>

using namespace GrumpyIRC;

static NetworkSession *meta_network_session_get(ScriptExtension *extension, unsigned int window_id)
{
    Scrollback *w = Scrollback::GetScrollbackByID(window_id);
    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": network_get_meta(window_id): unknown scrollback");
        return nullptr;
    }
    if (!w->GetSession())
    {
        GRUMPY_ERROR(extension->GetName() + ": network_get_meta(window_id): scrollback is not connected to IRC network");
        return nullptr;
    }

    return w->GetSession();
}

static libircclient::Network *meta_network_get(ScriptExtension *extension, unsigned int window_id)
{
    Scrollback *w = Scrollback::GetScrollbackByID(window_id);
    if (!w)
    {
        GRUMPY_ERROR(extension->GetName() + ": meta_network_get(window_id): unknown scrollback");
        return nullptr;
    }
    NetworkSession *session = w->GetSession();
    if (!session)
    {
        GRUMPY_ERROR(extension->GetName() + ": meta_network_get(window_id): scrollback is not connected to IRC network");
        return nullptr;
    }
    libircclient::Network *network = session->GetNetwork(w);
    if (!network)
    {
        GRUMPY_ERROR(extension->GetName() + ": meta_network_get(window_id): scrollback has NULL IRC network");
        return nullptr;
    }

    return network;
}

NetworkJS::NetworkJS(ScriptExtension *s) : GenericJSClass(s)
{

}

QHash<QString, QString> NetworkJS::GetFunctions()
{
    QHash<QString, QString> fh;
    fh.insert("get_nick", "(scrollback_id): return your nickname for network of scrollback");
    fh.insert("get_host", "(scrollback_id): return your host for network of scrollback");
    fh.insert("get_server_host", "(scrollback_id): return server hostname for network of scrollback");
    fh.insert("get_ident", "(scrollback_id): return your ident for network of scrollback");
    fh.insert("send_raw", "(scrollback_id, text): sends RAW data to network of scrollback");
    fh.insert("send_message", "(scrollback_id, target, message): sends a silent message to target for network of scrollback");
    fh.insert("get_network_name", "(scrollback_id): return network for network of scrollback");
    return fh;
}

QJSValue NetworkJS::get_nick(unsigned int scrollback_id)
{
    libircclient::Network *network = meta_network_get(this->script, scrollback_id);
    if (!network)
    {
        // Not existing
        return QJSValue(false);
    }
    return QJSValue(network->GetNick());
}

QJSValue NetworkJS::get_ident(unsigned int scrollback_id)
{
    libircclient::Network *network = meta_network_get(this->script, scrollback_id);
    if (!network)
    {
        // Not existing
        return QJSValue(false);
    }
    return QJSValue(network->GetIdent());
}

QJSValue NetworkJS::get_host(unsigned int scrollback_id)
{
    libircclient::Network *network = meta_network_get(this->script, scrollback_id);
    if (!network)
    {
        // Not existing
        return QJSValue(false);
    }
    return QJSValue(network->GetHost());
}

QJSValue NetworkJS::get_network_name(unsigned int scrollback_id)
{
    libircclient::Network *network = meta_network_get(this->script, scrollback_id);
    if (!network)
    {
        // Not existing
        return QJSValue(false);
    }
    return QJSValue(network->GetNetworkName());
}

QJSValue NetworkJS::get_server_host(unsigned int scrollback_id)
{
    libircclient::Network *network = meta_network_get(this->script, scrollback_id);
    if (!network)
    {
        // Not existing
        return QJSValue(false);
    }
    return QJSValue(network->GetServerAddress());
}

bool NetworkJS::send_raw(unsigned int scrollback_id, QString raw)
{
    Scrollback *w = Scrollback::GetScrollbackByID(scrollback_id);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": network_send_raw(window_id, text): unknown scrollback");
        return false;
    }
    if (!w->GetSession())
    {
        GRUMPY_ERROR(this->script->GetName() + ": network_send_raw(window_id, text): scrollback is not connected to IRC network");
        return false;
    }
    w->GetSession()->SendRaw(w, raw);
    return true;
}

bool NetworkJS::send_message(unsigned int scrollback_id, QString target, QString message)
{
    Scrollback *w = Scrollback::GetScrollbackByID(scrollback_id);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": network_send_message(window_id, target, text): unknown scrollback");
        return false;
    }
    if (!w->GetSession())
    {
        GRUMPY_ERROR(this->script->GetName() + ": network_send_message(window_id, target, text): scrollback is not connected to IRC network");
        return false;
    }
    w->GetSession()->SendMessage(w, target, message);
    return true;
}
