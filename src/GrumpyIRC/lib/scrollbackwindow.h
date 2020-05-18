//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2020

#ifndef SCROLLBACKWINDOW_H
#define SCROLLBACKWINDOW_H

#include <QDialog>

namespace Ui
{
    class ScrollbackWindow;
}

namespace GrumpyIRC
{
    class Scrollback;
    class ScrollbackFrame;

    //! This is a dedicated scrollback window that can be created outside of main window, provides regular scrollback
    class ScrollbackWindow : public QDialog
    {
            Q_OBJECT
        public:
            explicit ScrollbackWindow(QString name, QWidget *parent = nullptr);
            Scrollback *GetScrollback();
            ScrollbackFrame *GetScrollbackFrame() { return this->sf; }
            ~ScrollbackWindow();

        private:
            ScrollbackFrame *sf = nullptr;
            Ui::ScrollbackWindow *ui;
    };
}

#endif // SCROLLBACKWINDOW_H
