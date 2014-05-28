TEMPLATE = app
CONFIG += console
TARGET = dump-compressor
QMAKE_CXXFLAGS += -std=c++11
LIBS += -lz -llzma

HEADERS += DumpPlane.hpp
SOURCES += DumpPlane.cpp main.cpp
