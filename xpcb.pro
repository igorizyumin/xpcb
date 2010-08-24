#-------------------------------------------------
#
# Project created by QtCreator 2010-07-24T14:04:43
#
#-------------------------------------------------

QT       += core gui xml xmlpatterns

TARGET = xpcb
TEMPLATE = app


SOURCES += PCBView.cpp \
    GridToolbarWidget.cpp \
    AboutDialog.cpp \
    smcharacter.cpp \
    Shape.cpp \
    smfontutil.cpp \
    Part.cpp \
    Net.cpp \
    Trace.cpp \
    PCBObject.cpp \
    Text.cpp \
    Area.cpp \
    PCBDoc.cpp \
    Log.cpp \
    PolygonList.cpp \
    Polygon.cpp \
    Line.cpp \
	mainwindow.cpp

CONFIG(unittest) {
	QT += testlib
	SOURCES += xpcbtests/tst_XmlLoadTest.cpp \
				xpcbtests/testmain.cpp
} else {
	SOURCES += main.cpp
}

HEADERS  += PCBView.h \
    GridToolbarWidget.h \
    AboutDialog.h \
    global.h \
    smcharacter.h \
    Shape.h \
    smfontutil.h \
    Part.h \
    Net.h \
    Trace.h \
    PCBObject.h \
    Text.h \
    Area.h \
    PCBDoc.h \
    Log.h \
    PolygonList.h \
    Polygon.h \
    Line.h \
    mainwindow.h \
    xpcbtests/tst_XmlLoadTest.h

FORMS    += GridToolbarWidget.ui \
    AboutDialog.ui \
    mainwindow.ui

RESOURCES += \
    qtfreepcb.qrc

INCLUDEPATH += ../xpcb/polyboolean/
LIBS += -L../xpcb/polyboolean/ -lpolyboolean
