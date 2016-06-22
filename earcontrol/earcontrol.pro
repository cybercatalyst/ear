QT += core gui opengl

TARGET = earcontrol
TEMPLATE = app

QMAKE_CXXFLAGS -= -O2
QMAKE_CXXFLAGS += -O3
CONFIG -= console
CONFIG += flat

# On Windows we need the libraries provided in the 3rdparty folder.
win32 {
    LIBS += -L../3rdparty/fftw/lib \
            -L../3rdparty/jack/lib

    INCLUDEPATH += ../3rdparty/fftw/include \
                   ../3rdparty/jack/include
}

INCLUDEPATH += .

LIBS += -lfftw3

SOURCES += \
    digitalequalizer.cpp \
    fftwadapter.cpp \
    launcher.cpp \
    mainwindow.cpp \
    visualizerwidget.cpp \
    jnoise/jnoise.cpp \
    jnoise/randomgenerator.cpp \
    dspcore.cpp

HEADERS += \
    digitalequalizer.h \
    fftwadapter.h \
    launcher.cpp \
    mainwindow.h \
    Splash.png \
    visualizerwidget.h \
    jnoise/jnoise.h \
    jnoise/prbsgenerator.h \
    jnoise/randomgenerator.h \
    dspcore.h \
    semaphorelocker.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    Pics.qrc

include(../pods.pri)
