#-------------------------------------------------
#
# Project created by QtCreator 2010-07-24T14:04:43
#
#-------------------------------------------------

QT       += core gui

TARGET = xpcb
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    PCBView.cpp \
    GridToolbarWidget.cpp \
    AboutDialog.cpp \
    Netlist.cpp \
    PolyLine.cpp \
    gpc.cpp \
    utility.cpp

HEADERS  += MainWindow.h \
    PCBView.h \
    GridToolbarWidget.h \
    AboutDialog.h \
    Netlist.h \
    PolyLine.h \
    gpc.h \
    global.h \
    utility.h

FORMS    += MainWindow.ui \
    GridToolbarWidget.ui \
    AboutDialog.ui

RESOURCES += \
    qtfreepcb.qrc
