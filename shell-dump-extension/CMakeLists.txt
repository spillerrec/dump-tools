cmake_minimum_required(VERSION 3.0)


include_directories(zlib/include)
include_directories(lzma/include)

set(SOURCES
	ClassFactory.cpp
	DumpHandler.cpp
	DumpPlane.cpp
	dllmain.cpp
	Source.def
	)

add_library(shell-dump-extension SHARED ${SOURCES})

target_link_libraries(shell-dump-extension ../zlib/lib/zlibstatic)
target_link_libraries(shell-dump-extension ../lzma/bin/liblzma)
target_link_libraries(shell-dump-extension Windowscodecs.lib)