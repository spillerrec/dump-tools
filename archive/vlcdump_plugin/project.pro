TARGET  = $$qtLibraryTarget(vlcdump)
TEMPLATE = lib
CONFIG += plugin

SOURCES = vlcdump.cpp ImageEx.cpp Plane.cpp MultiPlaneIterator.cpp
HEADERS = vlcdump.h color.hpp ImageEx.hpp Plane.hpp MultiPlaneIterator.hpp
OTHER_FILES += dump.json