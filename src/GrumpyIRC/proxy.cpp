//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "proxy.h"
#include "grumpyconf.h"
#include <QNetworkProxy>
#include "ui_proxy.h"

using namespace GrumpyIRC;

static QNetworkProxy SetProxy(int type, QString host, unsigned int port, QString name, QString pass)
{
    QNetworkProxy proxy;
    switch (type)
    {
        case 0:
            QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
            return proxy;
        case 1:
            proxy.setType(QNetworkProxy::Socks5Proxy);
            break;
        case 2:
            proxy.setType(QNetworkProxy::HttpProxy);
            break;
        case 3:
            proxy.setType(QNetworkProxy::HttpCachingProxy);
            break;
        case 4:
            proxy.setType(QNetworkProxy::FtpCachingProxy);
            break;
        default:
            return proxy;
    }

    proxy.setHostName(host);
    proxy.setPort(port);
    proxy.setUser(name);
    proxy.setPassword(pass);
    QNetworkProxy::setApplicationProxy(proxy);
    return proxy;
}

Proxy::Proxy(QWidget *parent) : QDialog(parent), ui(new Ui::Proxy)
{
    this->ui->setupUi(this);
    this->ui->comboBox->addItem("None");
    this->ui->comboBox->addItem("Socks 5");
    this->ui->comboBox->addItem("Http");
    this->ui->comboBox->addItem("Http (caching proxy)");
    this->ui->comboBox->addItem("Ftp");
    this->ui->comboBox->setCurrentIndex(CONF->ProxyType());
    if (CONF->UsingProxy())
    {
        QNetworkProxy ps = CONF->GetProxy();
        this->ui->checkBox->setChecked(true);
        this->ui->lineEdit->setText(ps.hostName());
        this->ui->lineEdit_2->setText(QString::number(ps.port()));
        this->ui->lineEdit_3->setText(ps.user());
        this->ui->lineEdit_4->setText(ps.password());
    }
}

Proxy::~Proxy()
{
    delete this->ui;
}

void Proxy::on_buttonBox_accepted()
{
    QNetworkProxy proxy = SetProxy(this->ui->comboBox->currentIndex(), this->ui->lineEdit->text(), this->ui->lineEdit_2->text().toUInt(),
               this->ui->lineEdit_3->text(), this->ui->lineEdit_4->text());
    if (this->ui->checkBox->isChecked())
    {
        CONF->SetProxy(this->ui->comboBox->currentIndex());
        CONF->SetProxy(proxy);
    }
}

void Proxy::on_buttonBox_rejected()
{
    this->close();
}

void Proxy::on_comboBox_currentIndexChanged(int index)
{
    bool visible = index != 0;
    this->ui->label_5->setEnabled(visible);
    this->ui->label->setEnabled(visible);
    this->ui->label_4->setEnabled(visible);
    this->ui->label_3->setEnabled(visible);
    this->ui->lineEdit->setEnabled(visible);
    this->ui->lineEdit_2->setEnabled(visible);
    this->ui->lineEdit_3->setEnabled(visible);
    this->ui->lineEdit_4->setEnabled(visible);
}
