#-------------------------------------------------
#
# Project created by QtCreator 2020-10-10T15:48:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = realtimeplot
TEMPLATE = app

include(../../src/qmatplotwidget.pri)


SOURCES += main.cpp \
    widget.cpp

HEADERS  += \
    widget.h
