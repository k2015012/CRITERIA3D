#-------------------------------------------------
#
# CRITERIA3D distribution

# crop library
#
#-------------------------------------------------

QT      -= core gui

TARGET = crop
TEMPLATE = lib
CONFIG += staticlib

CONFIG += debug_and_release

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/crop
    } else {
        TARGET = release/crop
    }
}
win32:{
    TARGET = crop
}

INCLUDEPATH += ../mathFunctions ../crit3dDate ../soil

SOURCES += crop.cpp \
    root.cpp \
    development.cpp \
    biomass.cpp

HEADERS += crop.h \
    biomass.h \
    root.h \
    development.h
