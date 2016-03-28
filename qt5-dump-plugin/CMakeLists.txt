cmake_minimum_required(VERSION 3.0)

# C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Qt5Gui REQUIRED)

# Turn on warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


add_library(qt5-dump-plugin MODULE vlcdump.cpp Dump.cpp DumpPlane.cpp)
target_link_libraries(qt5-dump-plugin Qt5::Gui -lz -llzma)

install(TARGETS qt5-dump-plugin  DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/imageformats/)
#install(FILES dump.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/qimageioplugins/)
