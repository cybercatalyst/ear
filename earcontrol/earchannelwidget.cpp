/* This file is part of EAR, an audio processing tool.
 *
 * Copyright (C) 2011-2016 Otto Ritter, Jacob Dawid
 * otto.ritter.or@googlemail.com
 * jacob@omg-it.works
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Affero GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "earchannelwidget.h"
#include "ui_earchannelwidget.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QDebug>

EARChannelWidget::EARChannelWidget(EARFilter *earFilter, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EARChannelWidget),
    _earFilter(earFilter) {
    ui->setupUi(this);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(_equalizerWidget = new EqualizerWidget(earFilter->equalizer(), 20, 22050));
    ui->frame->setLayout(layout);

    setStyleSheet(
        "QProgressBar {"
            "border: 0px solid rgb(50, 50, 50);"
            "background-color: rgb(220,220,220);"
            "padding: 2px 2px;"
            "border-radius: 5px;"
        "}"

        "QProgressBar::chunk:vertical{ "
            "background-color: qlineargradient(spread:reflect, x1:1, y1:1, x2:0.204, y2:0.403, stop:0 rgba(86, 89, 85, 255), stop:1 rgba(0, 122, 180, 255));"
            "height: 1px; "
            "margin: 0.5px;"
        "}"
    );

    connect(_earFilter, SIGNAL(measuredSignalLevelChanged(int)), ui->progressBarIn, SLOT(setValue(int)));
    connect(_earFilter, SIGNAL(referenceSignalLevelChanged(int)), ui->progressBarRef, SLOT(setValue(int)));
    connect(_earFilter, SIGNAL(outputSignalLevelChanged(int)), ui->progressBarOut, SLOT(setValue(int)));

    connect(_earFilter, SIGNAL(calibrationFinished()), this, SLOT(calibrationFinished()));

    ui->pushButtonAutomaticAdaption->setChecked(_earFilter->automaticAdaptionActive());
    ui->pushButtonBypass->setChecked(_earFilter->bypassActive());
}

EARChannelWidget::~EARChannelWidget() {
    delete ui;
}

void EARChannelWidget::on_pushButtonAutomaticAdaption_clicked(bool on) {
    _earFilter->setAutomaticAdaptionActive(on);
}

void EARChannelWidget::on_comboBoxSignalSource_currentTextChanged(QString text) {
    if(text == "Reference input")
        _earFilter->setSignalSource(EARFilter::ExternalSource);
    if(text == "White noise")
        _earFilter->setSignalSource(EARFilter::WhiteNoise);
    if(text == "Pink noise")
        _earFilter->setSignalSource(EARFilter::PinkNoise);
}

void EARChannelWidget::calibrationFinished() {
    ui->pushButtonCalibrate->setChecked(false);
}

void EARChannelWidget::on_pushButtonCalibrate_clicked() {
    ui->pushButtonCalibrate->setChecked(true);
    _earFilter->startCalibration();
}

void EARChannelWidget::on_pushButtonBypass_clicked(bool on) {
    _earFilter->setBypassActive(on);
}




