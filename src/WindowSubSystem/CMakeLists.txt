cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WINDOW_SUBSYSTEM_SOURCES
	WindowSubSystemConfig.h
	WindowSubSystem.h
	WindowSubSystem.cpp
	Window.h
	Window.cpp
	HandlerContainer.hpp)

find_package(SDL2 REQUIRED)

add_library(
	WindowSubSystem STATIC
		${WINDOW_SUBSYSTEM_SOURCES})

link_directories(../hrs)
target_link_libraries(WindowSubSystem
	                    PRIVATE Hrs
						PRIVATE ${SDL2_LIBRARIES})
target_include_directories(WindowSubSystem PUBLIC ../)
