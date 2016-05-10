#-------------------------------------------------
#
# Project created by QtCreator 2016-05-10T17:56:45
#
#-------------------------------------------------

QT       += core gui
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    test.cpp \
    filesearchengine.cpp

HEADERS  += mainwindow.h \
    test.h \
    filesearchengine.h \
    filesearchengineitem.h

FORMS    += mainwindow.ui
