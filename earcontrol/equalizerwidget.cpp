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

#include "equalizerwidget.h"

#include <QGridLayout>
#include <QSlider>
#include <QDebug>
#include <QLabel>
#include <QPainter>

#include <cmath>

EqualizerWidget::EqualizerWidget(Equalizer *equalizer, int controls, int maxFrequency, QWidget *parent) :
    QWidget(parent),
    _equalizer(equalizer),
    _controls(controls),
    _maxFrequency(maxFrequency) {

    createSliders();

    _pollingTimer = new QTimer();
    _pollingTimer->setInterval(50);
    _pollingTimer->setSingleShot(true);

    connect(_pollingTimer, SIGNAL(timeout()), this, SLOT(poll()));
    _pollingTimer->start();
}

void EqualizerWidget::sliderMoved(int value) {
    QSlider *slider = dynamic_cast<QSlider*>(sender());
    if(slider) {
        //qDebug() << value << " for " << _sliders[_sliderIndices[slider]].controlFrequency;

    }
    update();
}

void EqualizerWidget::createSliders() {
    QGridLayout *layout = new QGridLayout();
    for(int i = 0; i <= _controls; i++) {
        FrequencySlider frequencySlider;

        frequencySlider.controlFrequency =
            (22050 *
             pow(10, ((double)i / (double)_controls) * 2.5 + 7.5) /
             pow(10, 10.0));

        frequencySlider.slider = new QSlider(Qt::Vertical);
        frequencySlider.slider->setMinimum(0);
        frequencySlider.slider->setMaximum(1000);

        frequencySlider.slider->setStyleSheet(
            QString(
                "QSlider::groove { background-color: %1; border-radius: 5px; width: 10px;}"
                "QSlider::handle { margin: 0 -2px; height: 14px; background: rgba(0,0,0,0.2); border-radius: 7px;}"
            ).arg("rgba(0, 0, 0, 0.1)")
        );

        connect(frequencySlider.slider, SIGNAL(valueChanged(int)), this, SLOT(sliderMoved(int)));

        frequencySlider.label = new QLabel(
            frequencySlider.controlFrequency < 1000 ?
                QString("%1").arg((int)frequencySlider.controlFrequency) :
                QString("%1k").arg(QString::number(frequencySlider.controlFrequency / 1000.0, 'f', 1))
            );

        frequencySlider.label->setAlignment(Qt::AlignHCenter);
        frequencySlider.label->setFont(QFont("Courier", 8));

//        frequencySlider.slider->setFixedWidth(fixedWidth);
        frequencySlider.label->setFixedWidth(30);

        layout->addWidget(frequencySlider.slider, 0, i);
        layout->addWidget(frequencySlider.label, 1, i);

        _sliders.append(frequencySlider);
        _sliderIndices.insert(frequencySlider.slider, i);
    }
    setLayout(layout);
}

void EqualizerWidget::poll() {
    double *c = _equalizer->controls();
    int numControls = _equalizer->numberOfControls();

    int eqStartRangeIndex = 0;
    int eqEndRangeIndex = 0;
    _equalizer->acquireControls();

    foreach(FrequencySlider fs, _sliders) {
        eqEndRangeIndex = fs.controlFrequency * numControls / _maxFrequency;

        double accSliderValues = 0.0;
        for(int i = eqStartRangeIndex; i < eqEndRangeIndex; i++) {
            accSliderValues += c[i];
        }

        double avgSliderValue = accSliderValues / (double)(eqEndRangeIndex - eqStartRangeIndex);
        fs.slider->setValue(avgSliderValue * fs.slider->maximum());
        eqStartRangeIndex = eqEndRangeIndex;
    }

    _equalizer->releaseControls();

    _pollingTimer->start();
}

void EqualizerWidget::paintEvent(QPaintEvent *paintEvent) {
    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing);
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(2);
    pen.setBrush(QColor(0, 0, 40, 120));
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    painter.setPen(pen);

    QPainterPath painterPath;

    int handleOffset = 2;
    QPoint point, lastPoint;
    foreach(FrequencySlider f, _sliders) {
        point = f.slider->pos() +
                QPoint(
                    f.slider->width() / 2,
                    f.slider->height() -
                    f.slider->value() * f.slider->height() /
                    f.slider->maximum() - handleOffset
                );

        if(f.slider == _sliders.first().slider) {
            painterPath.moveTo(point);
        } else {
            QPoint interpolX = QPoint((point.x() - lastPoint.x()) / 2, 0);
            painterPath.cubicTo(lastPoint + interpolX, point - interpolX, point);
        }

        lastPoint = point;
    }

    painter.drawPath(painterPath);


    QWidget::paintEvent(paintEvent);
}
