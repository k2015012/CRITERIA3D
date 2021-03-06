#-------------------------------------------------
#
# CRITERIA3D
# gis library
#
#-------------------------------------------------

QT  -= core gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += debug_and_release

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/gis
    } else {
        TARGET = release/gis
    }
}
win32:{
    TARGET = gis
}

INCLUDEPATH += ../mathFunctions

SOURCES += gis.cpp \
    gisIO.cpp \
    color.cpp \
    map.cpp

HEADERS += gis.h \
    color.h \
    gisIO.h \
    map.h


unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
