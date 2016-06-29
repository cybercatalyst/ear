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
    fftwadapter.cpp \
    launcher.cpp \
    mainwindow.cpp \
    jnoise/jnoise.cpp \
    jnoise/randomgenerator.cpp \
    dspcore.cpp \
    earfilter.cpp \
    earchannelwidget.cpp \
    equalizerwidget.cpp \
    equalizer.cpp

HEADERS += \
    fftwadapter.h \
    launcher.cpp \
    mainwindow.h \
    Splash.png \
    jnoise/jnoise.h \
    jnoise/prbsgenerator.h \
    jnoise/randomgenerator.h \
    dspcore.h \
    semaphorelocker.h \
    earfilter.h \
    earchannelwidget.h \
    equalizerwidget.h \
    equalizer.h

FORMS += \
    mainwindow.ui \
    earchannelwidget.ui

RESOURCES += \
    Pics.qrc

include(../pods.pri)
