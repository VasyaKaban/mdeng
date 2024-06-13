cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(WindowSubSystem STATIC)

target_sources(
	WindowSubSystem
	PRIVATE
		EventHandlers.h
		EventHandlers.cpp
		WindowSubSystem.h
		WindowSubSystem.cpp
		Window.h
		Window.cpp
)

find_package(SDL2)
find_package(Vulkan)

link_directories(../hrs)
target_link_libraries(
	WindowSubSystem
	PRIVATE SDL2::SDL2
	PRIVATE Hrs
)

target_include_directories(WindowSubSystem PUBLIC ../)
