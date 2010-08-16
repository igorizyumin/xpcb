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
    Net.cpp \
    Trace.cpp \
    PCBObject.cpp \
    Text.cpp \
    Area.cpp \
    PCBDoc.cpp \
    Log.cpp \
    PolygonList.cpp \
    Polygon.cpp

HEADERS  += MainWindow.h \
    PCBView.h \
    GridToolbarWidget.h \
    AboutDialog.h \
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
    Net.h \
    Trace.h \
    PCBObject.h \
    Text.h \
    Area.h \
    PCBDoc.h \
    Log.h \
    PolygonList.h \
    Polygon.h

FORMS    += MainWindow.ui \
    GridToolbarWidget.ui \
    AboutDialog.ui

RESOURCES += \
    qtfreepcb.qrc
