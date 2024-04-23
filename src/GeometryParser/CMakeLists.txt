cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WAVEFRONT_FOLDER_SOURCES
	WaveFront/ObjParserRange.h
	WaveFront/ObjParserRange.cpp
	WaveFront/ObjSchema.h
	WaveFront/ObjSchema.cpp
	WaveFront/ObjData.h
	WaveFront/ObjData.cpp
	WaveFront/Triangulator.h
	WaveFront/Triangulator.cpp
	WaveFront/Linerizer.h
	WaveFront/Linerizer.cpp
	WaveFront/NormalProcessing.h
	WaveFront/NormalProcessing.cpp)

add_library(
	GeometryParser STATIC
		${WAVEFRONT_FOLDER_SOURCES})

link_directories(../hrs)
target_link_libraries(GeometryParser PRIVATE Hrs)
target_include_directories(GeometryParser PUBLIC ../)
