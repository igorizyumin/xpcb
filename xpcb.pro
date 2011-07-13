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
    smfontutil.cpp \
    Part.cpp \
    Net.cpp \
    Trace.cpp \
    PCBObject.cpp \
    Text.cpp \
    Area.cpp \
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
    PinEditor.cpp \
    Plugin.cpp \
    FPPropDialog.cpp \
    main.cpp \
    EditLineDialog.cpp \
    LineEditor.cpp \
    SelectFPDialog.cpp \
    Footprint.cpp \
    Document.cpp \
    TraceEditor.cpp \
    SegmentLayerDialog.cpp

unittest {
	QT += testlib
	SOURCES +=	xpcbtests/tst_XmlLoadTest.cpp \
				xpcbtests/testmain.cpp \
	xpcbtests/tst_TextTest.cpp

	HEADERS += xpcbtests/tst_XmlLoadTest.h xpcbtests/tst_TextTest.h
} else {
	SOURCES +=
}

HEADERS  += PCBView.h \
    GridToolbarWidget.h \
    AboutDialog.h \
    global.h \
    smcharacter.h \
    smfontutil.h \
    Part.h \
    Net.h \
    Trace.h \
    PCBObject.h \
    Text.h \
    Area.h \
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
    PinEditor.h \
    Plugin.h \
    FPPropDialog.h \
    EditLineDialog.h \
    LineEditor.h \
    SelectFPDialog.h \
    Footprint.h \
    Document.h \
    TraceEditor.h \
    SegmentLayerDialog.h

FORMS    += GridToolbarWidget.ui \
    AboutDialog.ui \
    mainwindow.ui \
    ActionBar.ui \
    EditTextDialog.ui \
    SelFilterWidget.ui \
    EditPartDialog.ui \
    EditPadstackDialog.ui \
    ManagePadstacksDialog.ui \
    EditPinDialog.ui \
    FPPropDialog.ui \
    EditLineDialog.ui \
    SelectFPDialog.ui \
    SegmentLayerDialog.ui

RESOURCES += \
    qtfreepcb.qrc

INCLUDEPATH += ../xpcb/polyboolean/
LIBS += -L../xpcb/polyboolean/ -lpolyboolean
QMAKE_CXXFLAGS_DEBUG += -Wold-style-cast
