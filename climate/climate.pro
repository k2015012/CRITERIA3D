#-------------------------------------------------
#
# CRITERIA3D distribution
# climate library
#
#-------------------------------------------------

QT       -= gui
QT       += sql xml

TEMPLATE = lib
CONFIG += staticlib

CONFIG += debug_and_release


unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/climate
    } else {
        TARGET = release/climate
    }
}
win32:{
    TARGET = climate
}


INCLUDEPATH += ../crit3dDate ../mathFunctions ../gis ../utilities ../interpolation ../meteo ../dbMeteoPoints ../dbMeteoGrid

SOURCES += \
    climate.cpp \
    elaborationSettings.cpp \
    crit3dClimate.cpp \
    dbClimate.cpp \
    crit3dClimateList.cpp

HEADERS += \
    dbClimate.h \
    climate.h \
    elaborationSettings.h \
    crit3dClimate.h \
    crit3dClimateList.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
