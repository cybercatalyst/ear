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

#ifndef VISUALIZERWIDGET_H
#define VISUALIZERWIDGET_H

#include <QGLWidget>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QSemaphore>

#include "dspcore.h"

/**
  * @class VisualizerWidget
  * @brief View counterpart for the EAR processor.
  * This class is the view counterpart for the EAR processor,
  * which acts as the model class.
  */
class VisualizerWidget : public QGLWidget {
    Q_OBJECT
public:
    /**
      * Constructs a new view that will be attached to the given EAR processor.
      * @param earProcessor EAR processor to which this view will be attached.
      * @param parent Parent widget for the Qt framework.
      */
    VisualizerWidget(DSPCore& dspCore, QWidget *parent = 0);

protected:
    /** Reimplemented from QGLWidget. */
    void initializeGL();

    /**
      * Reimplemented from QGLWidget.
      * @param w Width of the new OpenGL viewport.
      * @param h Height of the new OpenGL viewport.
      */
    void resizeGL(int w, int h);

    /**
      * Reimplemented from QGLWidget.
      */
    void paintGL();

    /**
      * Reimplemented from QGLWidget.
      * @param wheelEvent Wheel event.
      */
    void wheelEvent(QWheelEvent *wheelEvent);

    /**
      * Reimplemented from QGLWidget.
      * @param mouseEvent Mouse event.
      */
    void mousePressEvent(QMouseEvent *mouseEvent);

    /**
      * Reimplemented from QGLWidget.
      * @param mouseEvent Mouse event.
      */
    void mouseReleaseEvent(QMouseEvent *mouseEvent);

    /**
      * Reimplemented from QGLWidget
      * @param mouseEvent Mouse event.
      */
    void mouseMoveEvent(QMouseEvent *mouseEvent);

private:
    /**
      * Draws a bezier line through all given control points.
      * @param controlPoints Array pointer to the first element of control points.
      * @param n Number of control points to use.
      */
    void drawBezier(GLfloat *controlPoints, int n);

    /** Model class this view is attached to. */
    DSPCore& _dspCore;

    /** Helper array to calculate frequency in logarithmic scale. */
    double m_logarithmicFrequencies[41];

    /** Helper array to determine which control points are centered at a logarithmic scale. */
    int m_centralControlPoints[41];

    /** Semaphore for multithreaded access to the zoom state. */
    QSemaphore *m_zoomSemaphore;

    /** Semaphore for multithreaded acces to the position state. */
    QSemaphore *m_positionSemaphore;

    /** Zoom state. */
    double m_zoom;

    /** Position state. */
    double m_position[2];

    /** Left mouse button down state. */
    bool m_leftMouseButtonDown;

    /** Last mouse position for stored for dragging. */
    double m_lastMousePosition[2];
};

#endif // VISUALIZERWIDGET_H
