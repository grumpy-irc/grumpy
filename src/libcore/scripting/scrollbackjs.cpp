//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2019

#include "scrollbackjs.h"
#include "scriptextension.h"
#include "../core.h"
#include "../configuration.h"
#include "../commandprocessor.h"
#include "../definitions.h"
#include "../exception.h"
#include "../eventhandler.h"
#include "../scrollback.h"
#include "../networksession.h"
#include <QHash>

using namespace GrumpyIRC;

ScrollbackJS::ScrollbackJS(ScriptExtension *s) : GenericJSClass(s)
{

}

QHash<QString, QString> GrumpyIRC::ScrollbackJS::GetFunctions()
{
    QHash<QString, QString> fh;
    fh.insert("list", "(): returns a list of all scrollback IDs");
    fh.insert("create", "(parent, name): creates a new scrollback, parent can be 0 if this should be root scrollback, "\
                        "returns false on error otherwise scrollback_id");
    fh.insert("remove", "(scrollback_id): destroy scrollback, can be only used for scrollbacks created with this script");
    fh.insert("write", "(scrollback_id, text): write text");
    fh.insert("get_type", "(scrollback_id): return type of scrollback; system, channel, user");
    fh.insert("get_target", "(scrollback_id): return target name of scrollback (channel name, user name)");
    fh.insert("request_network_reconnect", "(scrollback_id): reconnect a network that belongs to this scrollback");
    fh.insert("request_network_disconnect", "(scrollback_id): disconnects network that belongs to this scrollback");
    fh.insert("has_network", "(scrollback_id): return true if scrollback belongs to network");
    fh.insert("has_network_session", "(scrollback_id): returns true if scrollback has existing IRC session");
    return fh;
}

bool ScrollbackJS::write(unsigned int scrollback_id, QString text)
{
    Scrollback *w = Scrollback::GetScrollbackByID(scrollback_id);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": write(scrollback_id, text): unknown scrollback");
        return false;
    }
    w->InsertText(text);
    return true;
}

bool ScrollbackJS::has_network(unsigned int scrollback_id)
{
    Scrollback *w = Scrollback::GetScrollbackByID(scrollback_id);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": has_network(scrollback_id): unknown scrollback");
        return false;
    }
    if (!w->GetSession())
        return false;
    if (!w->GetSession()->GetNetwork(w))
        return false;

    return true;
}

bool ScrollbackJS::has_network_session(unsigned int scrollback_id)
{
    Scrollback *w = Scrollback::GetScrollbackByID(scrollback_id);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": has_network(scrollback_id): unknown scrollback");
        return false;
    }
    if (!w->GetSession())
        return false;

    return true;
}

QString ScrollbackJS::get_type(unsigned int scrollback_id)
{
    Scrollback *w = Scrollback::GetScrollbackByID(scrollback_id);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": get_type(scrollback_id): unknown scrollback");
        return "nullptr";
    }

    switch (w->GetType())
    {
        case ScrollbackType_Channel:
            return "channel";
        case ScrollbackType_System:
            return "system";
        case ScrollbackType_User:
            return "user";
    }
    return "unknown";
}

QJSValue ScrollbackJS::get_target(unsigned int scrollback_id)
{
    Scrollback *w = Scrollback::GetScrollbackByID(scrollback_id);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": get_type(scrollback_id): unknown scrollback");
        return QJSValue(false);
    }
    return QJSValue(w->GetTarget());
}

QJSValue ScrollbackJS::create(unsigned int parent_id, QString name)
{
    Scrollback *parent;
    if (!parent_id)
        parent = nullptr;
    else
        parent = Scrollback::GetScrollbackByID(parent_id);
    if (parent_id && !parent)
    {
        GRUMPY_ERROR(this->script->GetName() + ": create(parent, name): parent scrollback not found");
        return QJSValue(false);
    }

    Scrollback *w = Core::GrumpyCore->NewScrollback(parent, name, ScrollbackType_System);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": create(parent, name): unknown error");
        return QJSValue(false);
    }
    this->script->RegisterScrollback(w);

    return QJSValue(w->GetID());
}

bool ScrollbackJS::remove(unsigned int scrollback_id)
{
    Scrollback *w = Scrollback::GetScrollbackByID(scrollback_id);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": remove(parent, name): scrollback not found");
        return false;
    }

    if (!this->script->HasScrollback(w))
    {
        GRUMPY_ERROR(this->script->GetName() + ": remove(parent, name): scrollback doesn't belong to extension");
        return false;
    }

    this->script->DestroyScrollback(w);
    return true;
}

void ScrollbackJS::request_network_reconnect(unsigned int scrollback_id)
{
    Scrollback *w = Scrollback::GetScrollbackByID(scrollback_id);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": request_network_reconnect(scrollback): scrollback not found");
        return;
    }
    if (!w->GetSession())
    {
        GRUMPY_ERROR(this->script->GetName() + ": request_network_reconnect(scrollback): scrollback doesn't have a network");
        return;
    }
    w->GetSession()->RequestReconnect(w);
}

void ScrollbackJS::request_network_disconnect(unsigned int scrollback_id, QString reason)
{
    Scrollback *w = Scrollback::GetScrollbackByID(scrollback_id);
    if (!w)
    {
        GRUMPY_ERROR(this->script->GetName() + ": request_network_disconnect(scrollback): scrollback not found");
        return;
    }
    if (!w->GetSession())
    {
        GRUMPY_ERROR(this->script->GetName() + ": request_network_disconnect(scrollback): scrollback doesn't have a network");
        return;
    }
    w->GetSession()->RequestDisconnect(w, reason, false);
}

QList<int> ScrollbackJS::list()
{
    QList<int> scrollback_list;
    Scrollback::ScrollbackList_Mutex.lock();
    foreach (Scrollback *s, Scrollback::ScrollbackList)
    {
        scrollback_list.append((int)s->GetID());
    }
    Scrollback::ScrollbackList_Mutex.unlock();
    return scrollback_list;
}
