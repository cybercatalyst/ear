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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QTimerEvent>

#include "dspcore.h"

namespace Ui {
    class MainWindow;
}

/**
 * @class MainWindow
 * @author Jacob Dawid ( jacob@omg-it.works )
 * @author Otto Ritter ( otto.ritter.or@googlemail.com )
 * @date 09.2011-2016
 *
 * Main window for the whole application.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(DSPCore& dspCore, QWidget *parent = 0);
    ~MainWindow();

    void addEARChannel(QString channelName);

protected:
    /** Reimplemented from QWidget. */
    void closeEvent(QCloseEvent *closeEvent);

    void timerEvent(QTimerEvent *timerEvent);

private slots:

    /** Resets all equalizer controls to the highest possible value. */
    void resetControls();

    /** Action to load the left equalizer. */
    void loadLeftEqualizer();

    /** Action to load the right equalizer. */
    void loadRightEqualizer();

    /** Action to save the left equalizer. */
    void saveLeftEqualizer();

    /** Action to save the right equalizer. */
    void saveRightEqualizer();

private:
    /** Ui namespace for automatically generated GUI code. */
    Ui::MainWindow *ui;

    DSPCore& _dspCore;
};

#endif // MAINWINDOW_H
