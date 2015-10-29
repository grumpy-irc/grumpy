//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "connectwin.h"
#include "grumpyconf.h"
#include "mainwindow.h"
#include "ui_connectwin.h"
#include "../libirc/libirc/serveraddress.h"
#include "../libirc/libirc/irc_standards.h"
#include "../libgp/gp.h"

using namespace GrumpyIRC;

ConnectWin::ConnectWin(QWidget *parent) : QDialog(parent), ui(new Ui::ConnectWin)
{
    this->ui->setupUi(this);
    this->ui->comboBox->addItem("IRC");
    this->ui->comboBox->addItem("Grumpyd");
    this->ui->comboBox->setCurrentIndex(0);
    this->ui->lineEdit_2->setText(CONF->GetNick());
}

ConnectWin::~ConnectWin()
{
    delete this->ui;
}

void GrumpyIRC::ConnectWin::on_pushButton_clicked()
{
    if (this->ui->comboBox->currentIndex() == 0)
    {
        // Connect to IRC
        libirc::ServerAddress server(this->ui->comboBox_2->currentText());
        server.SetNick(this->ui->lineEdit_2->text());
        server.SetPassword(this->ui->lineEdit_3->text());
        server.SetPort(this->ui->lineEdit->text().toUInt());
        server.SetSSL(this->ui->checkBox->isChecked());
        MainWindow::Main->OpenServer(server);
    } else
    {
        // Connect to grumpy
        MainWindow::Main->OpenGrumpy(this->ui->comboBox_2->currentText(),
                                     this->ui->lineEdit->text().toInt(),
                                     this->ui->lineEdit_2->text(), this->ui->lineEdit_3->text(),
                                     this->ui->checkBox->isChecked());
    }
    this->close();
}

void GrumpyIRC::ConnectWin::on_comboBox_currentIndexChanged(int index)
{
    if (!this->ui->lineEdit->text().isEmpty())
    {
        // Don't change the text if user inserted their own port
        switch (this->ui->lineEdit->text().toInt())
        {
            case GP_DEFAULT_PORT:
            case GP_DEFAULT_SSL_PORT:
                return;
        }
    }
    if (this->ui->checkBox->isChecked())
    {
        if (index == 0)
            this->ui->lineEdit->setText(QString::number(IRC_STANDARD_PORT_SSL));
        else
            this->ui->lineEdit->setText(QString::number(GP_DEFAULT_SSL_PORT));
    } else
    {
        if (index == 0)
            this->ui->lineEdit->setText(QString::number(IRC_STANDARD_PORT));
        else
            this->ui->lineEdit->setText(QString::number(GP_DEFAULT_PORT));
    }
}
