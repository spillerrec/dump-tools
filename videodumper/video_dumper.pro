TEMPLATE = app
CONFIG += console c++14
TARGET = video_dumper
INCLUDEPATH += .
LIBS += -lavcodec -lavformat -lavutil

# Input
SOURCES += src/main.cpp
