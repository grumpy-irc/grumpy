//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include <QKeyEvent>
#include "skin.h"
#include "colorbox.h"
#include "ui_colorbox.h"

using namespace GrumpyIRC;

QPalette Color2Pal(QColor color)
{
    QPalette px;
    px.setColor(QPalette::Base, color);
    return px;
}

ColorBox::ColorBox(InputBox *in, QWidget *parent) : QDialog(parent),
    ui(new Ui::ColorBox)
{
    this->input = in;
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->move(this->input->pos());
    this->ui->setupUi(this);
    this->ui->label_1->setPalette(Skin::Current->Colors[1]);
    this->ui->label_2->setPalette(Skin::Current->Colors[2]);
    this->ui->label_3->setPalette(Skin::Current->Colors[3]);
    this->ui->label_4->setPalette(Skin::Current->Colors[4]);
    this->ui->label_5->setPalette(Skin::Current->Colors[5]);
    this->ui->label_6->setPalette(Skin::Current->Colors[6]);
    this->ui->label_7->setPalette(Skin::Current->Colors[7]);
    this->ui->label_8->setPalette(Skin::Current->Colors[8]);
    this->ui->label_9->setPalette(Skin::Current->Colors[9]);
    this->ui->label_10->setPalette(Skin::Current->Colors[10]);
    this->ui->label_11->setPalette(Skin::Current->Colors[11]);
    this->ui->label_12->setPalette(Skin::Current->Colors[12]);
    this->ui->label_13->setPalette(Skin::Current->Colors[13]);
    this->ui->label_14->setPalette(Skin::Current->Colors[14]);
    this->ui->label_15->setPalette(Skin::Current->Colors[15]);
}

ColorBox::~ColorBox()
{
    delete this->ui;
}

void ColorBox::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_1)
        this->input->InsertAtCurrentPosition("1");
    else if (e->key() == Qt::Key_2)
        this->input->InsertAtCurrentPosition("2");
    else if (e->key() == Qt::Key_3)
        this->input->InsertAtCurrentPosition("3");
    else if (e->key() == Qt::Key_4)
        this->input->InsertAtCurrentPosition("4");
    else if (e->key() == Qt::Key_5)
        this->input->InsertAtCurrentPosition("5");
    else if (e->key() == Qt::Key_6)
        this->input->InsertAtCurrentPosition("6");
    else if (e->key() == Qt::Key_7)
        this->input->InsertAtCurrentPosition("7");
    else if (e->key() == Qt::Key_8)
        this->input->InsertAtCurrentPosition("8");
    else if (e->key() == Qt::Key_9)
        this->input->InsertAtCurrentPosition("9");


    this->close();
}
