cmake_minimum_required(VERSION 3.0)

# C++14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

find_package(Qt5Core REQUIRED)

# Turn on warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


add_executable(video_dumper src/main.cpp )
target_link_libraries(video_dumper Qt5::Core -lavcodec -lavformat -lavutil)
install(TARGETS video_dumper RUNTIME DESTINATION bin)
