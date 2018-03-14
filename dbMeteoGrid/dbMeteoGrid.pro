#-------------------------------------------------
#
# Project created by QtCreator 2018-03-06T13:25:19
#
#-------------------------------------------------

QT       += network sql xml

QT       -= gui

TARGET = dbMeteoGrid

TEMPLATE = lib
CONFIG += staticlib
DEFINES += DBMETEOGRID_LIBRARY

INCLUDEPATH += ../mathFunctions ../gis ../crit3dDate  ../meteo ../utilities

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
        dbMeteoGrid.cpp \

HEADERS += \
        dbMeteoGrid.h
