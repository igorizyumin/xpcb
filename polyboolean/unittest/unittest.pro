#-------------------------------------------------
#
# Project created by QtCreator 2010-08-15T11:02:22
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_PLineTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -L../ -lpolyboolean
INCLUDEPATH += ../

SOURCES += tst_PLineTest.cpp \
    main.cpp \
    PAreaTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    AutoTest.h
