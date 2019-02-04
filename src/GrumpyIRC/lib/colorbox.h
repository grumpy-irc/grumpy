//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef COLORBOX_H
#define COLORBOX_H

#include "grumpy_global.h"
#include <QDialog>
#include "inputbox.h"

namespace Ui
{
    class ColorBox;
}

namespace GrumpyIRC
{
    class Skin;
    class LIBGRUMPYSHARED_EXPORT ColorBox : public QDialog
    {
            Q_OBJECT

        public:
            explicit ColorBox(InputBox *in, QWidget *parent = nullptr);
            ~ColorBox() override;
            InputBox *input;

        protected:
            void keyPressEvent(QKeyEvent *e) override;

        private:
            Ui::ColorBox *ui;
    };
}

#endif // COLORBOX_H
