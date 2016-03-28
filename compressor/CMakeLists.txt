cmake_minimum_required(VERSION 3.0)

# C++14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Qt5Core REQUIRED)
find_package(Qt5Concurrent REQUIRED)

# Turn on warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


add_executable(dump-compressor DumpPlane.cpp main.cpp)
target_link_libraries(dump-compressor Qt5::Core Qt5::Concurrent -lz -llzma)
install(TARGETS dump-compressor RUNTIME DESTINATION bin)
