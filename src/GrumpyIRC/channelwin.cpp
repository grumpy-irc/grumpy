//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "skin.h"
#include "../libcore/networksession.h"
#include "scrollbackframe.h"
#include "../libirc/libircclient/mode.h"
#include "../libirc/libircclient/channel.h"
#include "channelwin.h"
#include "ui_channelwin.h"

using namespace GrumpyIRC;

ChannelWin::ChannelWin(NetworkSession *session, libircclient::Network *network, libircclient::Channel *channel, ScrollbackFrame *parent) : QDialog(parent), ui(new Ui::ChannelWin)
{
    this->ui->setupUi(this);
    this->ignore = true;
    this->_ns = session;
    this->updateTopic = false;
    this->ui->plainTextEdit->setPlainText(channel->GetTopic());
    this->_network = network;
    this->_channel = channel;
    this->ignore = false;
    this->ui->groupBox->setTitle("Topic set by " + channel->GetTopicUser() + " at " + channel->GetTopicTime().toString());
    this->ui->plainTextEdit->setPalette(Skin::GetDefault()->Palette());
}

ChannelWin::~ChannelWin()
{
    delete this->ui;
}

void GrumpyIRC::ChannelWin::on_pushButton_clicked()
{
    if (this->updateTopic)
        this->_ns->SendRaw(((ScrollbackFrame*)this->parent())->GetScrollback(), "TOPIC " + this->_channel->GetName() + " :" + this->ui->plainTextEdit->toPlainText());
    this->close();
}

void GrumpyIRC::ChannelWin::on_plainTextEdit_textChanged()
{
    if (this->ignore)
        return;
    this->updateTopic = true;
}
