//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "mainwindow.h"
#include "scrollbacklist.h"
#include "grumpyeventhandler.h"
#include "grumpyconf.h"
#include "packetsnifferwin.h"
#include "hooks.h"
#include "messagebox.h"
#include "grumpydcfwin.h"
#include "skin.h"
#include "ui_scrollbacklist.h"
#include "scrollbackframe.h"
#include "scrollbacksmanager.h"
#include "scrollbacklist_node.h"
#include <QAction>
#include <QMenu>
#include <QDebug>
#include <libcore/core.h>
#include <libcore/exception.h>
#include <libcore/generic.h>
#include <libcore/networksession.h>
#include <libcore/ircsession.h>
#include <libcore/grumpydsession.h>

using namespace GrumpyIRC;

ScrollbackList *ScrollbackList::scrollbackList = NULL;

ScrollbackList *ScrollbackList::GetScrollbackList()
{
    if (!scrollbackList)
        throw new NullPointerException("ScrollbackList * ScrollbackList::scrollbackList", BOOST_CURRENT_FUNCTION);
    return scrollbackList;
}

ScrollbackList::ScrollbackList(QWidget *parent) : QDockWidget(parent), ui(new Ui::ScrollbackList)
{
    if (scrollbackList)
        throw new Exception("You can't create multiple instances of this class in same moment", BOOST_CURRENT_FUNCTION);

    scrollbackList = this;
    this->timer = new QTimer();
    connect (this->timer, SIGNAL(timeout()), this, SLOT(OnUpdate()));
    this->ui->setupUi(this);
    this->ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    this->model = new QStandardItemModel(0, 1, this);
    this->root = this->model->invisibleRootItem();
    this->ui->treeView->setModel(this->model);
    //this->ui->treeView->setColumnHidden(1, true);
    this->ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->treeView->setHeaderHidden(true);
    this->ui->treeView->setPalette(Skin::GetCurrent()->Palette());
    this->timer->start(2000);
}

ScrollbackList::~ScrollbackList()
{
    delete this->timer;
    delete this->ui;
    scrollbackList = NULL;
}

void GrumpyIRC::ScrollbackList::RegisterHidden()
{
    foreach (ScrollbackFrame *scrollback, ScrollbackFrame::ScrollbackFrames)
    {
        if (scrollback->IsHidden())
        {
            ScrollbackList_Node *parent = NULL;
            if (scrollback->GetParent())
                parent = scrollback->GetParent()->TreeNode;

            this->RegisterWindow(scrollback, parent);
        }
    }
}

void ScrollbackList::RegisterWindow(ScrollbackFrame *scrollback, ScrollbackList_Node *parent_node)
{
    QStandardItem *root;
    if (parent_node != NULL)
        root = parent_node;
    else
        root = this->GetRootTreeItem();
    ScrollbackList_Node *node = new ScrollbackList_Node(scrollback);
    node->IsSystem = scrollback->IsSystem;
    scrollback->TreeNode = node;
    root->appendRow(node);
    this->ui->treeView->expand(root->index());
    //this->ui->treeView->sortByColumn(0);
    root->sortChildren(0);
}

QStandardItem *ScrollbackList::GetRootTreeItem()
{
    return this->root;
}

void ScrollbackList::UnregisterHidden()
{
    foreach(ScrollbackFrame *scrollback, ScrollbackFrame::ScrollbackFrames)
    {
        if (scrollback->IsHidden())
        {
            ScrollbackList_Node *parent = NULL;
            if (scrollback->GetParent())
                parent = scrollback->GetParent()->TreeNode;
            this->UnregisterWindow(scrollback->TreeNode, parent);
            scrollback->TreeNode = NULL;
        }
    }
}

void ScrollbackList::UnregisterWindow(ScrollbackList_Node *node, ScrollbackList_Node *parent_n)
{
    QModelIndex index = this->model->indexFromItem(node);
    QModelIndex parent;
    if (parent_n != NULL)
        parent = this->model->indexFromItem(parent_n);
    this->model->removeRow(index.row(), parent);
}

void ScrollbackList::OnUpdate()
{
    foreach (ScrollbackList_Node *node, ScrollbackList_Node::NodesList)
    {
        if ((node->GetScrollback()->LastMenuTooltipUpdate.secsTo(QDateTime::currentDateTime())) > 20)
        {
            Scrollback *scrollback = node->GetScrollback()->GetScrollback();
            if (!scrollback)
                return;
            node->GetScrollback()->LastMenuTooltipUpdate = QDateTime::currentDateTime();
            node->UpdateToolTip();
            if (scrollback->IsDead() && scrollback->GetType() == ScrollbackType_System)
            {
                // This is a disconnected network most likely, let's reconnect it
                if (!scrollback->GetSession())
                    return;
                // Reconnect the network if we want to do that
                if (scrollback->GetSession()->IsAutoreconnect(scrollback))
                {
                    scrollback->GetSession()->RequestReconnect(scrollback);
                    if (scrollback->GetSession()->GetType() == SessionType_Grumpyd)
                    {
                        // This function will start immediately destroying scrollbacks
                        // if we stay in the loop we will get in troubles as next windows
                        // may be already deleted by then
                        return;
                    }
                }
            }
        }
    }
}

void GrumpyIRC::ScrollbackList::on_treeView_activated(const QModelIndex &index)
{
    UiHooks::OnInput();
    this->switchWindow(index);
}

void GrumpyIRC::ScrollbackList::on_treeView_customContextMenuRequested(const QPoint &pos)
{
    UiHooks::OnInput();
    ScrollbackFrame *wx = this->selectedWindow();
    // We have some window selected so let's display a menu
    QPoint globalPos = this->ui->treeView->viewport()->mapToGlobal(pos);
    QMenu Menu;
    // Items
    QAction *menuClose = NULL;
    QAction *menuInsrFavorites = NULL;
    QAction *menuAuto = NULL;
    QAction *menuAway_Set = NULL;
    QAction *menuAway_Uns = NULL;
    QAction *menuSettings = NULL;
    QAction *menuPart = NULL;
    QAction *menuJoin = NULL;
    QAction *menuHide = NULL;
    QAction *menuReconnect = NULL;
    QAction *menuDisconnect = NULL;
    QAction *menuSniffer = NULL;
    QAction *menuJoinAll = NULL;
    QAction *menuSound = NULL;
    QAction *menuNotify = NULL;
    QAction *menuDeaf = NULL;

    if (wx)
    {
        menuClose = new QAction(QObject::tr("Close"), &Menu);
        Menu.addAction(menuClose);
        menuHide = new QAction(QObject::tr("Hide window"), &Menu);
        menuHide->setEnabled(wx->GetScrollback()->IsHidable());
        menuHide->setCheckable(true);
        menuHide->setChecked(wx->IsHidden());
        Menu.addAction(menuHide);

        if (!wx->IsDeletable || (!wx->IsDead() && wx->GetScrollback()->GetType() != ScrollbackType_User))
            menuClose->setEnabled(false);

        if (wx->IsNetwork())
        {
            menuInsrFavorites = new QAction(QObject::tr("Insert to favorite networks"), &Menu);
            Menu.addAction(menuInsrFavorites);
            menuAuto = new QAction(QObject::tr("Automatically reconnect"), &Menu);
            menuAuto->setCheckable(true);
            if (wx->GetSession())
                menuAuto->setChecked(wx->GetSession()->IsAutoreconnect(wx->GetScrollback()));
            Menu.addAction(menuAuto);
            menuAway_Set = new QAction(QObject::tr("Set away"), &Menu);
            menuAway_Uns = new QAction("Unset away", &Menu);
            Menu.addAction(menuAway_Uns);
            Menu.addAction(menuAway_Set);
        }

        if (wx->IsGrumpy())
        {
            menuSettings = new QAction(QObject::tr("Settings"), &Menu);
            Menu.addAction(menuSettings);
        }

        if (wx->IsChannel())
        {
            Menu.addSeparator();
            if (wx->IsDead())
            {
                menuJoin = new QAction(QObject::tr("Join"), &Menu);
                Menu.addAction(menuJoin);
            }
            menuPart = new QAction(QObject::tr("Part"), &Menu);
            // Check if we can part the chan
            if (wx->IsDead())
                menuPart->setEnabled(false);
            Menu.addAction(menuPart);
            menuJoinAll = new QAction(QObject::tr("Rejoin all dead channels on this network"), &Menu);
            Menu.addAction(menuJoinAll);
        }
        if (wx->IsNetwork())
        {
            Menu.addSeparator();
            if (wx->IsDead())
            {
                menuReconnect = new QAction(QObject::tr("Reconnect"), &Menu);
                Menu.addAction(menuReconnect);
            }
            menuDisconnect = new QAction(QObject::tr("Disconnect"), &Menu);
            Menu.addAction(menuDisconnect);
            menuDisconnect->setEnabled(!wx->IsDead());
            menuSniffer = new QAction(QObject::tr("Network sniffer"), &Menu);
            Menu.addAction(menuSniffer);
        }
        Menu.addSeparator();
    }
    QAction *showHidden = new QAction(QObject::tr("Show hidden windows"), &Menu);
    showHidden->setCheckable(true);
    Menu.addAction(showHidden);
    showHidden->setChecked(this->ShowHidden);

    if (wx)
    {
        menuNotify = new QAction(QObject::tr("Notify on message"), &Menu);
        menuNotify->setCheckable(true);
        menuNotify->setChecked(wx->GetScrollback()->GetPropertyAsBool("notify"));
        Menu.addAction(menuNotify);
        menuDeaf = new QAction(QObject::tr("Mute"), &Menu);
        menuDeaf->setCheckable(true);
        menuDeaf->setChecked(wx->Muted);
        Menu.addAction(menuDeaf);
    }

    QAction* selectedItem = Menu.exec(globalPos);
    if (!selectedItem)
        return;
    if (selectedItem == menuClose)
    {
        this->closeWindow();
    } else if (selectedItem == menuPart)
    {
        wx->RequestPart();
    } else if (selectedItem == menuSettings)
    {
        if (wx->IsDead())
        {
            MessageBox::Display("settings_not_connected_network", "Error", "You can't use this function on disconnected window", MainWindow::Main);
            return;
        }
        GrumpydCfWin *window = new GrumpydCfWin((GrumpydSession*)wx->GetSession());
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->show();
    } else if (selectedItem == menuDisconnect)
    {
        wx->RequestDisconnect();
    } else if (selectedItem == menuSniffer)
    {
        this->sniffer(wx);
    } else if (selectedItem == menuJoin)
    {
        wx->RequestJoin();
    } else if (selectedItem == menuReconnect)
    {
        wx->Reconnect();
    } else if (selectedItem == menuAway_Set)
    {
        if (wx->GetSession())
            wx->GetSession()->SendRaw(wx->GetScrollback(), "AWAY :" + CONF->GetDefaultAwayReason());
    } else if (selectedItem == menuAway_Uns)
    {
        if (wx->GetSession())
            wx->GetSession()->SendRaw(wx->GetScrollback(), "AWAY");
    } else if (selectedItem == menuHide)
    {
        wx->ToggleHide();
    } else if (selectedItem == menuJoinAll)
    {
        IRCSession *irc_session = NULL;
        if (wx->GetSession()->GetType() == SessionType_Grumpyd)
        {
            GrumpydSession *session = (GrumpydSession*)wx->GetSession();
            irc_session = session->GetSessionFromWindow(wx->GetScrollback());
        } else if (wx->GetSession()->GetType() == SessionType_IRC)
        {
            irc_session = (IRCSession*)wx->GetSession();
        }
        if (!irc_session)
            throw new NullPointerException("irc_session", BOOST_CURRENT_FUNCTION);
        foreach (Scrollback *channel, irc_session->GetChannelScrollbacks())
        {
            if (channel->IsDead())
                wx->TransferRaw("JOIN " + channel->GetTarget(), libircclient::Priority_Low);
        }
    } else if (selectedItem == menuAuto)
    {
        if (wx->GetSession())
            wx->GetSession()->SetAutoreconnect(wx->GetScrollback(), menuAuto->isChecked());
    } else if (selectedItem == showHidden)
    {
        this->ShowHidden = showHidden->isChecked();
        if (this->ShowHidden)
            RegisterHidden();
        else
            UnregisterHidden();
    } else if (selectedItem == menuNotify)
    {
        wx->SetProperty("notify", menuNotify->isChecked());
    } else if (selectedItem == menuDeaf)
    {
        wx->Muted = !wx->Muted;
    }
}

void ScrollbackList::switchWindow(const QModelIndex &index)
{
    if (this->model->itemFromIndex(index) == this->root)
        return;
    ScrollbackList_Node *node = (ScrollbackList_Node*)this->model->itemFromIndex(index);
    if (!node)
        return;
    MainWindow::Main->GetScrollbackManager()->SwitchWindow(node->GetScrollback());
}

void ScrollbackList::sniffer(ScrollbackFrame *window)
{
    NetworkSession *session = window->GetSession();
    if (!session)
        return;
    PacketSnifferWin *wx = new PacketSnifferWin();
    wx->setAttribute(Qt::WA_DeleteOnClose);
    if (session->GetType() == SessionType_IRC)
    {
        wx->Load((IRCSession*)session);
    }
    else if (session->GetType() == SessionType_Grumpyd)
    {
        GrumpydSession *gs = (GrumpydSession*)session;
        IRCSession *is = gs->GetSessionFromWindow(window->GetScrollback());
        if (is == nullptr)
        {
            GRUMPY_ERROR("NULL network");
            delete wx;
            return;
        }
        wx->Load(gs, is);
    } else
    {
        GRUMPY_ERROR("Unknown session type");
    }
    wx->show();
}

void ScrollbackList::closeWindow()
{
    QModelIndex index = this->ui->treeView->currentIndex();
    ScrollbackList_Node *node = (ScrollbackList_Node*)this->model->itemFromIndex(index);
    if (!node)
        return;
    if (node->GetScrollback()->IsGrumpy() && node->GetScrollback()->GetScrollback() != node->GetScrollback()->GetSession()->GetSystemWindow() && node->GetScrollback()->IsSystem)
    {
        // User wants to close a network on remote Grumpyd, which is dangerous, slow and irreversible
        if (MessageBox::Question("grumpyd-network-close", "Close window", "Closing a network window on remote bouncer, will delete all history and all channels"\
                                 " permanently from the database. This is not reversible. For large networks, this may take long time."\
                                 " Are you sure you want to do that?") != MessageBoxResponse_Yes)
            return;
    }
    node->GetScrollback()->RequestClose();
}

ScrollbackFrame *ScrollbackList::selectedWindow()
{
    QModelIndex index = this->ui->treeView->currentIndex();
    ScrollbackList_Node *node = (ScrollbackList_Node*)this->model->itemFromIndex(index);
    if (node == 0)
        return NULL;
    return node->GetScrollback();
}

void GrumpyIRC::ScrollbackList::on_treeView_clicked(const QModelIndex &index)
{
    UiHooks::OnInput();
    this->switchWindow(index);
}
