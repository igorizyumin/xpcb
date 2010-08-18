#-------------------------------------------------
#
# Project created by QtCreator 2010-08-15T08:49:48
#
#-------------------------------------------------

QT       -= gui

TARGET = polyboolean
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    triamono.cpp \
    triacons.cpp \
    polybool.cpp \
    PLine.cpp \
    pbsweep.cpp \
    pbio.cpp \
    pbgeom.inl \
    pbgeom.cpp \
    PArea.cpp

HEADERS += \
    Sort.h \
    polybool.h \
    PLine.h \
    pbtria.h \
    pbsweep.h \
    pbio.h \
    pbimpl.h \
    pbgeom.h \
    pbdefs.h \
    PArea.h \
    ObjHeap.h
