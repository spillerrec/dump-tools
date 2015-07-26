TARGET  = $$qtLibraryTarget(dump)
TEMPLATE = lib
CONFIG += plugin
LIBS += -lz -llzma
QMAKE_CXXFLAGS += -std=c++11 #required way for Qt4!

SOURCES = vlcdump.cpp Dump.cpp DumpPlane.cpp
HEADERS = vlcdump.hpp Dump.hpp DumpPlane.hpp
OTHER_FILES += dump.json