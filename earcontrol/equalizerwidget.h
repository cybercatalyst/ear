#ifndef EQUALIZERWIDGET_H
#define EQUALIZERWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QList>
#include <QMap>

#include <QPaintEvent>
#include <QTimer>

#include "equalizer.h"

class EqualizerWidget : public QWidget {
    Q_OBJECT
public:
    explicit EqualizerWidget(Equalizer *equalizer, int controls, int maxFrequency, QWidget *parent = 0);

signals:

public slots:
    void sliderMoved(int value);

protected:
    void paintEvent(QPaintEvent *paintEvent);

private slots:
    void poll();

private:
    void createSliders();

    struct FrequencySlider {
        QSlider *slider;
        QLabel *label;
        double controlFrequency;
    };

    Equalizer *_equalizer;
    QList<FrequencySlider> _sliders;
    QMap<QSlider*, int> _sliderIndices;

    QTimer *_pollingTimer;

    int _controls;
    int _maxFrequency;
};

#endif // EQUALIZERWIDGET_H
