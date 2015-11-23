//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SCROLLBACKSMANAGER_H
#define SCROLLBACKSMANAGER_H

#include "../libcore/definitions.h"
#include <QFrame>
#include <QList>

namespace Ui
{
    class ScrollbacksManager;
}

namespace GrumpyIRC
{
    class Scrollback;
    class ScrollbackFrame;

    /*!
     * \brief The ScrollbacksManager class is used to manage all scrollback windows we have in application, you can create new windows
     *        or change the existing ones.
     */
    class ScrollbacksManager : public QFrame
    {
            Q_OBJECT
        public:
            static ScrollbacksManager *Global;
            explicit ScrollbacksManager(QWidget *parent = 0);
            ~ScrollbacksManager();
            /*!
             * \brief CreateWindow Makes a new scrollback window, this window must be destroyed only by calling destroy window
             * \param name
             * \param focus
             * \return
             */
            ScrollbackFrame *CreateWindow(QString name, ScrollbackFrame *parent = NULL, bool focus = false, bool is_deletable = true, Scrollback *scrollback = NULL, bool is_system = false);
            ScrollbackFrame *GetWindowFromID(scrollback_id_t id);
            ScrollbackFrame *GetWindowFromScrollback(Scrollback *scrollback);
            void DestroyWindow(ScrollbackFrame *window);
            void SwitchWindow(scrollback_id_t id);
            void SwitchWindow(ScrollbackFrame *window);
            ScrollbackFrame *GetCurrentScrollback() const;

        private:
            ScrollbackFrame *currentWidget;
            QList<ScrollbackFrame*> Scrollbacks;
            Ui::ScrollbacksManager *ui;
    };
}

#endif // SCROLLBACKSMANAGER_H
