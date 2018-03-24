//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "../libcore/grumpydsession.h"
#include "../libcore/generic.h"
#include "grumpyeventhandler.h"
#include "grumpydusercfgwin.h"
#include "grumpyconf.h"
#include "grumpydcfwin.h"
#include "messagebox.h"
#include "ui_grumpydcfwin.h"
#include <QMenu>

using namespace GrumpyIRC;

GrumpydCfWin::GrumpydCfWin(GrumpydSession *session, QWidget *parent) : QDialog(parent), ui(new Ui::GrumpydCfWin)
{
    ui->setupUi(this);
    this->GrumpySession = session;
    this->ui->checkBox->setChecked(this->getBool("offline_ms_bool", true));
    this->ui->lineEdit_3->setText(this->getString("nick", "GrumpydUser"));
    this->ui->lineEdit_4->setText(this->getString("ident", "g"));
    this->ui->lineEdit_7->setText(this->getString("user", "GrumpyChat"));
    this->ui->lineEdit_5->setText(this->getString("quit_message", "https://github.com/grumpy-irc/grumpy"));
    this->ui->lineEdit_6->setText(QString::number(this->getUInt("maximum_bsize", 2000)));
    this->ui->lineEdit_2->setText(QString::number(this->getUInt("initial_bsize", 80)));
    this->ui->plainTextEdit->setPlainText(this->getString("session_on_conn_raw", "AWAY"));
    this->ui->plainTextEdit_2->setPlainText(this->getString("session_on_disc_raw", "AWAY :" + CONF->GetDefaultAwayReason()));
    this->ui->lineEdit->setText(this->getString("offline_ms_text", this->ui->lineEdit->text()));
    this->timer = new QTimer();
    connect(this->timer, SIGNAL(timeout()), this, SLOT(OnRefresh()));
}

GrumpydCfWin::~GrumpydCfWin()
{
    delete this->timer;
    delete this->ui;
}

void GrumpyIRC::GrumpydCfWin::on_buttonBox_accepted()
{
    this->set("offline_ms_bool", QVariant(this->ui->checkBox->isChecked()));
    this->set("offline_ms_text", QVariant(this->ui->lineEdit->text()));
    this->set("default_nick", this->ui->lineEdit_3->text());
    this->set("default_ident", this->ui->lineEdit_4->text());
    this->set("nick", this->ui->lineEdit_3->text());
    this->set("ident", this->ui->lineEdit_4->text());
    this->set("user", this->ui->lineEdit_7->text());
    this->set("quit_message", this->ui->lineEdit_5->text());
    this->set("session_on_conn_raw", this->ui->plainTextEdit->toPlainText());
    this->set("session_on_disc_raw", this->ui->plainTextEdit_2->toPlainText());
    this->set("initial_bsize", this->ui->lineEdit_2->text().toUInt());
    this->set("maximum_bsize", this->ui->lineEdit_6->text().toUInt());
    QHash<QString, QVariant> hash;
    hash.insert("merge", this->GrumpySession->Preferences);
    //this->set("away_msg", QVariant(this->ui->lineEdit_2->text()));
    this->GrumpySession->SendProtocolCommand(GP_CMD_OPTIONS, hash);
}

template <typename T>
void GrumpydCfWin::set(QString key, T value)
{
    if (this->GrumpySession->Preferences.contains(key))
        this->GrumpySession->Preferences[key] = value;
    else
        this->GrumpySession->Preferences.insert(key, value);
}

QString GrumpydCfWin::getString(QString key, QString missing)
{
    if (this->GrumpySession->Preferences.contains(key))
        return this->GrumpySession->Preferences[key].toString();
    return missing;
}

unsigned int GrumpyIRC::GrumpydCfWin::getUInt(QString key, unsigned int default_uint)
{
    if (this->GrumpySession->Preferences.contains(key))
        return this->GrumpySession->Preferences[key].toUInt();
    return default_uint;
}

bool GrumpydCfWin::getBool(QString key, bool default_bool)
{
    if (this->GrumpySession->Preferences.contains(key))
        return this->GrumpySession->Preferences[key].toBool();
    return default_bool;
}

void GrumpydCfWin::ClearUserList()
{
    this->ui->tableUser->clear();
    QStringList heading;
    heading << "User" << "Is locked" << "Is online" << "Grumpyd sessions" << "IRC sessions" << "Role";
    this->ui->tableUser->verticalHeader()->setVisible(false);
    this->ui->tableUser->setColumnCount(heading.size());
    this->ui->tableUser->setHorizontalHeaderLabels(heading);
    this->ui->tableUser->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->tableUser->setShowGrid(false);
    this->ui->tableUser->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    while (this->ui->tableUser->rowCount() > 0)
        this->ui->tableUser->removeRow(0);
}

void GrumpydCfWin::RefreshUserList()
{
    this->ClearUserList();
    this->GrumpySession->SendProtocolCommand(GP_CMD_SYS_LIST_USER);
    this->userLoaded = true;
    if (!this->timer->isActive())
        this->timer->start(100);
}

void GrumpyIRC::GrumpydCfWin::on_tabWidget_currentChanged(int index)
{
    if (index == 1 && !this->userLoaded)
        this->RefreshUserList();
}

void GrumpydCfWin::OnRefresh()
{
    if (this->lastKnownRefresh == this->GrumpySession->GetLastUpdateOfUserList())
        return;

    this->lastKnownRefresh = this->GrumpySession->GetLastUpdateOfUserList();
    QList<QVariant> users = this->GrumpySession->GetUserList();
    this->ClearUserList();
    this->uid.clear();
    int row = 0;
    foreach (QVariant user_info, users)
    {
        QHash<QString, QVariant> info = user_info.toHash();
        if (!info.contains("name") || !info.contains("id"))
        {
            continue;
        }
        this->ui->tableUser->insertRow(row);
        this->ui->tableUser->setItem(row, 0, new QTableWidgetItem(info["name"].toString()));
        this->uid.insert(info["name"].toString(), info["id"].toUInt());
        if (info.contains("locked"))
            this->ui->tableUser->setItem(row, 1, new QTableWidgetItem(Generic::Bool2String(info["locked"].toBool())));
        if (info.contains("online"))
            this->ui->tableUser->setItem(row, 2, new QTableWidgetItem(Generic::Bool2String(info["online"].toBool())));
        if (info.contains("gp_session_count"))
            this->ui->tableUser->setItem(row, 3, new QTableWidgetItem(QString::number(info["gp_session_count"].toInt())));
        if (info.contains("irc_session_count"))
            this->ui->tableUser->setItem(row, 4, new QTableWidgetItem(QString::number(info["irc_session_count"].toInt())));
        if (info.contains("role"))
            this->ui->tableUser->setItem(row, 5, new QTableWidgetItem(info["role"].toString()));
    }

    // Make rows a bit smaller for easier reading
    this->ui->tableUser->resizeRowsToContents();
}

void GrumpyIRC::GrumpydCfWin::on_tableUser_customContextMenuRequested(const QPoint &position)
{
    QMenu Menu;
    QPoint p = this->ui->tableUser->viewport()->mapToGlobal(position);
    QAction *menuAdd = new QAction(QObject::tr("Create user"), &Menu);
    Menu.addAction(menuAdd);
    Menu.addSeparator();
    QAction *menuLock = new QAction("Lock", &Menu);
    Menu.addAction(menuLock);
    QAction *menuUnlock = new QAction("Unlock", &Menu);
    Menu.addAction(menuUnlock);
    Menu.addSeparator();
    QAction *menuRemove = new QAction(QObject::tr("Remove"), &Menu);
    Menu.addAction(menuRemove);

    QAction* selectedItem = Menu.exec(p);
    if (!selectedItem)
        return;

    if (selectedItem == menuAdd)
    {
        GrumpydUserCfgWin *new_user = new GrumpydUserCfgWin(this->GrumpySession, this);
        new_user->show();
    } else if (selectedItem == menuRemove)
    {
        int selected_row = this->ui->tableUser->currentRow();
        QString user_name = this->ui->tableUser->item(selected_row, 0)->text();
        user_id_t user_id = this->uid[user_name];
        MessageBoxResponse response = MessageBox::Question("grumpyd-delete-user", "Remove user", "Do you really want to delete user " + user_name + "?", this);
        if (response != MessageBoxResponse_Yes)
            return;
        QHash<QString, QVariant> user;
        user.insert("id", user_id);
        this->GrumpySession->SendProtocolCommand(GP_CMD_SYS_REMOVE_USER, user);
        this->GrumpySession->SendProtocolCommand(GP_CMD_SYS_LIST_USER);
    } else if (selectedItem == menuLock)
    {
        int selected_row = this->ui->tableUser->currentRow();
        QString user_name = this->ui->tableUser->item(selected_row, 0)->text();
        user_id_t user_id = this->uid[user_name];
        MessageBoxResponse response = MessageBox::Question("grumpyd-lock-user", "Lock user", "Do you really want to lock user " + user_name + "? This will also disconnect them from all their grumpy sessions. "\
                                                           "IRC sessions will remain active, but user will not be able to control them.", this);
        if (response != MessageBoxResponse_Yes)
            return;
        QHash<QString, QVariant> user;
        user.insert("id", user_id);
        this->GrumpySession->SendProtocolCommand(GP_CMD_SYS_LOCK_USER, user);
        this->GrumpySession->SendProtocolCommand(GP_CMD_SYS_LIST_USER);
    } else if (selectedItem == menuUnlock)
    {
        int selected_row = this->ui->tableUser->currentRow();
        QString user_name = this->ui->tableUser->item(selected_row, 0)->text();
        user_id_t user_id = this->uid[user_name];
        QHash<QString, QVariant> user;
        user.insert("id", user_id);
        this->GrumpySession->SendProtocolCommand(GP_CMD_SYS_UNLOCK_USER, user);
        this->GrumpySession->SendProtocolCommand(GP_CMD_SYS_LIST_USER);
    }
}
