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
    utility.cpp \
    PartList.cpp \
    smcharacter.cpp \
    Shape.cpp \
    UndoList.cpp \
    smfontutil.cpp \
    Part.cpp \
    DesignRules.cpp \
    TextList.cpp \
    Net.cpp

HEADERS  += MainWindow.h \
    PCBView.h \
    GridToolbarWidget.h \
    AboutDialog.h \
    Netlist.h \
    PolyLine.h \
    gpc.h \
    global.h \
    utility.h \
    PartList.h \
    smcharacter.h \
    Shape.h \
    UndoList.h \
    smfontutil.h \
    Part.h \
    DesignRules.h \
    TextList.h \
    Net.h

FORMS    += MainWindow.ui \
    GridToolbarWidget.ui \
    AboutDialog.ui

RESOURCES += \
    qtfreepcb.qrc
