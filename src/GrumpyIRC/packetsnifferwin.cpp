//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QDateTime>
#include "../libirc/libircclient/network.h"
#include "../libcore/ircsession.h"
#include "skin.h"
#include "packetsnifferwin.h"
#include "ui_packetsnifferwin.h"

using namespace GrumpyIRC;

PacketSnifferWin::PacketSnifferWin(QWidget *parent) : QDialog(parent), ui(new Ui::PacketSnifferWin)
{
    this->ui->setupUi(this);
    this->ui->plainTextEdit->clear();
    this->ui->plainTextEdit->setReadOnly(true);
    this->ui->plainTextEdit->setPalette(Skin::GetCurrent()->Palette());
    this->ui->plainTextEdit->setFont(Skin::GetCurrent()->TextFont);
}

PacketSnifferWin::~PacketSnifferWin()
{
    delete this->ui;
}

void PacketSnifferWin::Load(IRCSession *session)
{
    QString text;
    QList<NetworkSniffer_Item*> list = session->GetSniffer();
    foreach (NetworkSniffer_Item *snif, list)
    {
        QString direction = " < ";
        if (snif->_outgoing)
            direction =     " > ";
        text += snif->Time.toString() + " " + session->GetNetwork()->GetServerAddress() + direction + snif->Text;
    }
    this->ui->plainTextEdit->appendPlainText(text);
}
