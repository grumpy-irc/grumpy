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
#include "../libirc/libircclient/network.h"
#include "../libirc/libircclient/channel.h"
#include "channelwin.h"
#include "ui_channelwin.h"

using namespace GrumpyIRC;

ChannelWin::ChannelWin(NetworkSession *session, libircclient::Network *network, libircclient::Channel *channel, ScrollbackFrame *parent) : QDialog(parent), ui(new Ui::ChannelWin)
{
    this->chanmode = new libircclient::Mode();
    this->ui->setupUi(this);
    this->ignore = true;
    this->_ns = session;
    this->updateTopic = false;
    this->ui->plainTextEdit->setPlainText(channel->GetTopic());
    this->_network = network;
    this->_channel = channel;
    this->setWindowTitle(channel->GetName());
    this->ignore = false;
    this->ui->groupBox->setTitle("Topic set by " + channel->GetTopicUser() + " at " + channel->GetTopicTime().toString());
    this->ui->plainTextEdit->setPalette(Skin::GetDefault()->Palette());

    // Mode view
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setVisible(false);
    this->ui->tableWidget->setColumnCount(2);
    this->ui->tableWidget->setShowGrid(false);
    this->ui->tableWidget->setColumnWidth(0, 60);
    this->ui->tableWidget->setColumnWidth(1, 500);
    //this->ui->tableWidget->setVerticalScrollMode(Q);

    int row = 0;
    foreach (char mode, network->GetCModes())
    {
        this->ui->tableWidget->insertRow(row);
        QCheckBox *modeBox = new QCheckBox(this);
        modeBox->setText(QString(QChar(mode)));
        connect(modeBox, SIGNAL(clicked(bool)), this, SLOT(OnMode(bool)));
        this->checkBoxesMode.append(modeBox);
        if (channel->GetMode().Includes(mode))
            modeBox->setChecked(true);
        this->ui->tableWidget->setCellWidget(row, 0, modeBox);
        this->ui->tableWidget->setItem(row, 1, new QTableWidgetItem(network->GetHelpForMode(mode, "Unknown mode, refer IRC manual (/raw help)")));
        row++;
    }
    this->ui->tableWidget->resizeRowsToContents();

    // Bans
    QStringList heading_1;
    heading_1 << "Ban" << "Set on" << "Set by";
    this->ui->tableWidget_2->verticalHeader()->setVisible(false);
    this->ui->tableWidget_2->setColumnCount(heading_1.size());
    this->ui->tableWidget_2->setShowGrid(false);
    QList<libircclient::ChannelPMode> list_b = channel->GetBans();
    this->ui->tableWidget_2->setHorizontalHeaderLabels(heading_1);
    int ib = 0;
    foreach (libircclient::ChannelPMode ban, list_b)
    {
        this->ui->tableWidget_2->insertRow(ib);
        this->ui->tableWidget_2->setItem(ib, 0, new QTableWidgetItem(ban.Parameter));
        this->ui->tableWidget_2->setItem(ib, 1, new QTableWidgetItem(ban.SetOn.toString()));
        this->ui->tableWidget_2->setItem(ib, 2, new QTableWidgetItem(ban.SetBy.ToString()));
        ib++;
    }

    this->ui->tableWidget_2->resizeColumnsToContents();
    this->ui->tableWidget_2->resizeRowsToContents();

    // Exceptions
    QStringList heading_2;
    heading_2 << "Exception" << "Set on" << "Set by";
    this->ui->tableWidget_3->verticalHeader()->setVisible(false);
    this->ui->tableWidget_3->setColumnCount(heading_2.size());
    this->ui->tableWidget_3->setHorizontalHeaderLabels(heading_2);
    this->ui->tableWidget_3->setShowGrid(false);
    QList<libircclient::ChannelPMode> list_e = channel->GetExceptions();
    int ie = 0;
    foreach (libircclient::ChannelPMode exception, list_e)
    {
        this->ui->tableWidget_3->insertRow(ie);
        this->ui->tableWidget_3->setItem(ie, 0, new QTableWidgetItem(exception.Parameter));
        this->ui->tableWidget_3->setItem(ie, 1, new QTableWidgetItem(exception.SetOn.toString()));
        this->ui->tableWidget_3->setItem(ie, 2, new QTableWidgetItem(exception.SetBy.ToString()));
        ie++;
    }
    this->ui->tableWidget_3->resizeColumnsToContents();
    this->ui->tableWidget_3->resizeRowsToContents();

    // Invites
    QStringList heading_in;
    heading_in << "Invite" << "Set on" << "Set by";
    this->ui->tableWidget_4->verticalHeader()->setVisible(false);
    this->ui->tableWidget_4->setColumnCount(heading_in.size());
    this->ui->tableWidget_4->setHorizontalHeaderLabels(heading_in);
    this->ui->tableWidget_4->setShowGrid(false);
    QList<libircclient::ChannelPMode> list_i = channel->GetExceptions();
    ie = 0;
    foreach (libircclient::ChannelPMode in, list_i)
    {
        this->ui->tableWidget_4->insertRow(ie);
        this->ui->tableWidget_4->setItem(ie, 0, new QTableWidgetItem(in.Parameter));
        this->ui->tableWidget_4->setItem(ie, 1, new QTableWidgetItem(in.SetOn.toString()));
        this->ui->tableWidget_4->setItem(ie, 2, new QTableWidgetItem(in.SetBy.ToString()));
        ie++;
    }

    this->ui->tableWidget_4->resizeColumnsToContents();
    this->ui->tableWidget_4->resizeRowsToContents();

    this->headings();
}

ChannelWin::~ChannelWin()
{
    delete this->chanmode;
    delete this->ui;
}

void GrumpyIRC::ChannelWin::on_pushButton_clicked()
{
    if (this->updateTopic)
        this->_ns->SendRaw(((ScrollbackFrame*)this->parent())->GetScrollback(), "TOPIC " + this->_channel->GetName() + " :" + this->ui->plainTextEdit->toPlainText());
    if (!this->chanmode->IsEmpty())
        this->_ns->SendRaw(((ScrollbackFrame*)this->parent())->GetScrollback(), "MODE " + this->_channel->GetName() + " " + this->chanmode->ToString());
    this->close();
}

void GrumpyIRC::ChannelWin::on_plainTextEdit_textChanged()
{
    if (this->ignore)
        return;
    this->updateTopic = true;
}

void ChannelWin::OnMode(bool toggled)
{
    QCheckBox *checkBox = (QCheckBox*)QObject::sender();
    if (toggled)
        this->chanmode->SetMode("+" + checkBox->text());
    else
        this->chanmode->SetMode("-" + checkBox->text());
}

void GrumpyIRC::ChannelWin::on_tableWidget_2_customContextMenuRequested(const QPoint &pos)
{

}

void GrumpyIRC::ChannelWin::on_tableWidget_3_customContextMenuRequested(const QPoint &pos)
{

}

void GrumpyIRC::ChannelWin::on_tableWidget_4_customContextMenuRequested(const QPoint &pos)
{

}

void ChannelWin::headings()
{
    this->ui->tab_2->setWindowTitle(QObject::tr("Bans") + QString(" (") + QString::number(this->ui->tableWidget_2->rowCount()) + ")");
    this->ui->tab_3->setWindowTitle(QObject::tr("Exceptions") + " (" + QString::number(this->ui->tableWidget_3->rowCount()) + ")");
    this->ui->tab_4->setWindowTitle(QObject::tr("Invites") + " (" + QString::number(this->ui->tableWidget_4->rowCount()) + ")");
}
