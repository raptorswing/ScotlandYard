#-------------------------------------------------
#
# Project created by QtCreator 2014-02-22T16:16:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScotlandYard
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

RESOURCES += \
    resources.qrc

win32: RC_FILE = winicon.rc
