#----------------------------------------------------------------
#
#   criteriaModel library
#
#   Water balance 1D
#   algorithms for soil water infiltration, redistribution,
#   capillary rise, crop water demand and irrigation.
#
#   This library is part of CRITERIA3D distribution
#
#----------------------------------------------------------------

QT      += core sql xml
QT      -= gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += debug_and_release

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/criteriaModel
    } else {
        TARGET = release/criteriaModel
    }
}
win32:{
    TARGET = criteriaModel
}

INCLUDEPATH +=  ../crit3dDate ../mathFunctions ../gis ../meteo \
                ../dbMeteoGrid ../soil ../crop ../utilities

SOURCES += \
    criteria1DCase.cpp \
    criteria1DMeteo.cpp \
    criteria1DSimulation.cpp \
    criteria1DUnit.cpp \
    water1D.cpp

HEADERS += \
    criteria1DCase.h \
    criteria1DMeteo.h \
    criteria1DSimulation.h \
    criteria1DUnit.h \
    water1D.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

