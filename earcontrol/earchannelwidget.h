#ifndef EARCHANNELWIDGET_H
#define EARCHANNELWIDGET_H

#include <QWidget>
#include <QTimer>

#include "earfilter.h"
#include "equalizerwidget.h"

namespace Ui {
class EARChannelWidget;
}

class EARChannelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EARChannelWidget(EARFilter *earFilter, QWidget *parent = 0);
    ~EARChannelWidget();

public slots:
    void on_pushButtonAutomaticAdaption_clicked(bool on);
    void on_pushButtonCalibrate_clicked();
    void on_pushButtonBypass_clicked(bool on);

    void on_comboBoxSignalSource_currentTextChanged(QString text);

private slots:
    void calibrationFinished();

private:
    Ui::EARChannelWidget *ui;

    QTimer *_updateGUITimer;

    EqualizerWidget *_equalizerWidget;

    EARFilter *_earFilter;
};

#endif // EARCHANNELWIDGET_H
