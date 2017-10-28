cmake_minimum_required(VERSION 3.0)

find_package(Qt5Core REQUIRED)

# Turn on warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


add_executable(video_dumper src/main.cpp )
target_link_libraries(video_dumper Qt5::Core -lavcodec -lavformat -lavutil)
install(TARGETS video_dumper RUNTIME DESTINATION bin)


# Enable C++14 features
set_property(TARGET video_dumper PROPERTY CXX_STANDARD 14)
set_property(TARGET video_dumper PROPERTY CXX_STANDARD_REQUIRED ON)
