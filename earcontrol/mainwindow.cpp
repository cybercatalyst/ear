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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

#define MUSIC_COMBO_TEXT "Ext. Source"
#define WHITE_NOISE_COMBO_TEXT "White Noise"
#define PINK_NOISE_COMBO_TEXT "Pink Noise"
#define FILE_TYPES "*.csv"

MainWindow::MainWindow(DSPCore &dspCore, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _dspCore(dspCore) {
    ui->setupUi(this);
    showMaximized();


    m_visualizer = new VisualizerWidget(_dspCore, this);
    setCentralWidget(m_visualizer);

    m_updateTimer.setInterval(1000 / 50);
    m_updateTimer.setSingleShot(false);
    m_updateTimer.start();
    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateGUI()));

    ui->signalSourceComboBox->addItem(MUSIC_COMBO_TEXT);
    ui->signalSourceComboBox->addItem(WHITE_NOISE_COMBO_TEXT);
    ui->signalSourceComboBox->addItem(PINK_NOISE_COMBO_TEXT);

    connect(ui->signalSourceComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(signalSourceChanged(QString)));
    connect(ui->resetControlsPushButton, SIGNAL(clicked()), this, SLOT(resetControls()));

    connect(ui->actionLoadLeft, SIGNAL(triggered()), this, SLOT(loadLeftEqualizer()));
    connect(ui->actionSaveLeft, SIGNAL(triggered()), this, SLOT(saveLeftEqualizer()));
    connect(ui->actionLoadRight, SIGNAL(triggered()), this, SLOT(loadRightEqualizer()));
    connect(ui->actionSaveRight, SIGNAL(triggered()), this, SLOT(saveRightEqualizer()));

    connect(ui->automaticAdaption, SIGNAL(toggled(bool)),
            &_dspCore, SLOT(setAutomaticAdaptionActive(bool)));
    connect(ui->bypass, SIGNAL(toggled(bool)),
            &_dspCore, SLOT(setBypassActive(bool)));
    connect(ui->calibrateAction, SIGNAL(triggered()),
            &_dspCore, SLOT(calibrate()));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *closeEvent) {
    _dspCore.client().deactivate();
    QMainWindow::closeEvent(closeEvent);
}

void MainWindow::updateGUI() {
    ui->microphoneLeftChannelLevel->setValue(_dspCore.microphoneLevelLeft());
    ui->microphoneRightChannelLevel->setValue(_dspCore.microphoneLevelRight());
    ui->musicSourceLeftChannelLevel->setValue(_dspCore.signalSourceLevelLeft());
    ui->musicSourceRightChannelLevel->setValue(_dspCore.signalSourceLevelRight());
    ui->statusbar->showMessage(QString("Server statistics: CPU Load: %1% - Sample Rate: %2 Hz - Buffer Size: %3 Samples")
                               .arg((int)_dspCore.client().cpuLoad())
                               .arg(_dspCore.client().sampleRate())
                               .arg(_dspCore.client().bufferSize()));
    m_visualizer->updateGL();
}

void MainWindow::signalSourceChanged(QString text) {
    if(text == MUSIC_COMBO_TEXT)
        _dspCore.setSignalSource(DSPCore::ExternalSource);
    if(text == WHITE_NOISE_COMBO_TEXT)
        _dspCore.setSignalSource(DSPCore::WhiteNoise);
    if(text == PINK_NOISE_COMBO_TEXT)
        _dspCore.setSignalSource(DSPCore::PinkNoise);
}

void MainWindow::resetControls() {
    DigitalEqualizer *leftEqualizer = _dspCore.leftEqualizer();
    DigitalEqualizer *rightEqualizer = _dspCore.rightEqualizer();
    double *controls;

    leftEqualizer->acquireControls();
    controls = leftEqualizer->controls();
    for(int i = 0; i < 2048; i++)
        controls[i] = 0.01;
    leftEqualizer->releaseControls();

    rightEqualizer->acquireControls();
    controls = rightEqualizer->controls();
    for(int i = 0; i < 2048; i++)
        controls[i] = 0.01;
    rightEqualizer->releaseControls();
}

void MainWindow::loadLeftEqualizer() {
    QString homeLocation = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
    QString fileName = QFileDialog::getOpenFileName(this, "Load Left Equalizer", homeLocation, FILE_TYPES);
    if(!_dspCore.leftEqualizer()->loadControlsFromFile(fileName)) {
        QMessageBox::warning(this, "Error Loading File", "There was an error loading the specified file.");
    }
}

void MainWindow::loadRightEqualizer() {
    QString homeLocation = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
    QString fileName = QFileDialog::getOpenFileName(this, "Load Right Equalizer", homeLocation, FILE_TYPES);
    if(!_dspCore.rightEqualizer()->loadControlsFromFile(fileName)) {
        QMessageBox::warning(this, "Error Loading File", "There was an error loading the specified file.");
    }
}

void MainWindow::saveLeftEqualizer() {
    QString homeLocation = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
    QString fileName = QFileDialog::getSaveFileName(this, "Save Left Equalizer", homeLocation, FILE_TYPES);
    if(!_dspCore.leftEqualizer()->saveControlsToFile(fileName)) {
        QMessageBox::warning(this, "Error Saving File", "There was an error saving the specified file.");
    }
}

void MainWindow::saveRightEqualizer() {
    QString homeLocation = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
    QString fileName = QFileDialog::getSaveFileName(this, "Save Right Equalizer", homeLocation, FILE_TYPES);
    if(!_dspCore.rightEqualizer()->saveControlsToFile(fileName)) {
        QMessageBox::warning(this, "Error Saving File", "There was an error saving the specified file.");
    }
}
