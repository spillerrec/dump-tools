TARGET  = $$qtLibraryTarget(vlcdump)
TEMPLATE = lib
CONFIG += plugin
QMAKE_CXXFLAGS += -std=c++11
LIBS += -lz -llzma

SOURCES = vlcdump.cpp Dump.cpp DumpPlane.cpp
HEADERS = vlcdump.hpp Dump.hpp DumpPlane.hpp
OTHER_FILES += dump.json