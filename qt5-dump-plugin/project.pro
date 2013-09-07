TARGET  = $$qtLibraryTarget(vlcdump)
TEMPLATE = lib
CONFIG += plugin
QMAKE_CXXFLAGS += -std=c++11
LIBS += -lz

SOURCES = vlcdump.cpp Dump.cpp
HEADERS = vlcdump.hpp Dump.hpp
OTHER_FILES += dump.json