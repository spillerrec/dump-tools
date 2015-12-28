TEMPLATE = app
CONFIG += console
TARGET = video_dumper
INCLUDEPATH += .
QMAKE_CXXFLAGS += -std=c++14
LIBS += -lavcodec -lavformat -lavutil

# Input
SOURCES += src/main.cpp
