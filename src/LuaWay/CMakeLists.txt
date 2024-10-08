cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(LuaWay STATIC)

target_sources(
	LuaWay
	PRIVATE
	    Common.h
		Stack.h
		VmType.h
		VmType.cpp
		NativeType.h
		NativeType.cpp
		Status.h
		Status.cpp
		FunctionResult.h
		RefIterator.h
		RefIterator.cpp
		Ref.h
		Ref.cpp
		Register.hpp
		VMBase.h
		VMBase.cpp
		VM.h
		VM.cpp
		VMView.h
		VMView.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LUAJIT REQUIRED IMPORTED_TARGET luajit)

link_directories(../hrs)
target_link_libraries(
	LuaWay
	PRIVATE ${LUAJIT_LINK_LIBRARIES}
	PRIVATE Hrs
)

target_include_directories(LuaWay PUBLIC ../)
target_include_directories(LuaWay PUBLIC ${LUAJIT_INCLUDE_DIRS})