//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef PROXY_H
#define PROXY_H

#include "grumpy_global.h"
#include <QDialog>

namespace Ui
{
    class Proxy;
}

namespace GrumpyIRC
{
    //! Proxy
    class LIBGRUMPYSHARED_EXPORT Proxy : public QDialog
    {
            Q_OBJECT
        public:
            static void Init();
            explicit Proxy(QWidget *parent = nullptr);
            void Enable(bool b);
            ~Proxy();

        private slots:
            void on_buttonBox_accepted();
            void on_buttonBox_rejected();
            void on_comboBox_currentIndexChanged(int index);

        private:
            Ui::Proxy *ui;
    };
}

#endif // PROXY_H
