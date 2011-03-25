#-------------------------------------------------
#
# Project created by QtCreator 2010-07-24T14:04:43
#
#-------------------------------------------------

QT       += core gui xml xmlpatterns

CONFIG(unittest) {
	message(Building unit tests.)
	TARGET = xpcb-test
} else {
	message(Building app executable)
	TARGET = xpcb
}

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
	mainwindow.cpp \
    ActionBar.cpp \
    Controller.cpp \
    Editor.cpp \
    EditTextDialog.cpp \
    SelFilterWidget.cpp \
    LayerWidget.cpp \
    EditPartDialog.cpp \
	PartEditor.cpp \
    global.cpp \
    EditPadstackDialog.cpp \
    ManagePadstacksDialog.cpp \
    EditPinDialog.cpp \
    PinEditor.cpp

unittest {
	QT += testlib
	SOURCES +=	xpcbtests/tst_XmlLoadTest.cpp \
				xpcbtests/testmain.cpp \
	xpcbtests/tst_TextTest.cpp

	HEADERS += xpcbtests/tst_XmlLoadTest.h xpcbtests/tst_TextTest.h
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
    ActionBar.h \
    Controller.h \
    Editor.h \
    EditTextDialog.h \
    SelFilterWidget.h \
    LayerWidget.h \
    EditPartDialog.h \
	PartEditor.h \
    EditPadstackDialog.h \
    ManagePadstacksDialog.h \
    EditPinDialog.h \
    PinEditor.h

FORMS    += GridToolbarWidget.ui \
    AboutDialog.ui \
    mainwindow.ui \
    ActionBar.ui \
    EditTextDialog.ui \
    SelFilterWidget.ui \
    EditPartDialog.ui \
    EditPadstackDialog.ui \
    ManagePadstacksDialog.ui \
    EditPinDialog.ui

RESOURCES += \
    qtfreepcb.qrc

INCLUDEPATH += ../xpcb/polyboolean/
LIBS += -L../xpcb/polyboolean/ -lpolyboolean
