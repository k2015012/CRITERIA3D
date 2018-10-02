#-----------------------------------------------------
#
# soilFluxes3D
#
# numerical solution for flow equations
# of water and heat in the soil
# in a three-dimensional domain
#
# this library is part of CRITERIA3D distribution
#
#-----------------------------------------------------

QT   -= core gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += debug_and_release

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/soilFluxes3D
    } else {
        TARGET = release/soilFluxes3D
    }
}
win32:{
    TARGET = soilFluxes3D
}

SOURCES +=  \
    boundary.cpp \
    balance.cpp \
    water.cpp \
    solver.cpp \
    memory.cpp \
    soilPhysics.cpp \
    soilFluxes3D.cpp \
    heat.cpp \
    extra.cpp


HEADERS += \
    header/types.h \
    header/parameters.h \
    header/boundary.h \
    header/balance.h \
    header/water.h \
    header/solver.h \
    header/memory.h \
    header/soilPhysics.h \
    header/soilFluxes3D.h \
    header/extra.h \
    header/heat.h
