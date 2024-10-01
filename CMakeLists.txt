cmake_minimum_required(VERSION 3.28)

project(mdeng LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src/hrs)
add_subdirectory(src/Renderer)
add_subdirectory(src/LuaWay)
#add_subdirectory(src/GeometryParser)
#add_subdirectory(src/WindowSubSystem)

#add_custom_target(generated_loader ALL)

#add_custom_command(
#	TARGET generated_loader
#	PRE_BUILD
#	COMMAND lua
#	ARGS
#	    ${CMAKE_CURRENT_SOURCE_DIR}/gen_loader.lua Loader Device
#		${CMAKE_CURRENT_SOURCE_DIR}/src/out.h
#		vkQueueSubmit vkCreateFence vkCreateSemaphore vkCreateBuffer vkCreateImage vkCreateSwapchainKHR)

add_executable(${PROJECT_NAME})

target_sources(
${PROJECT_NAME}
    PUBLIC
	    src/main.cpp
)

#add_dependencies(${PROJECT_NAME} generated_loader)

#if(MSVC)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
#else()
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -g")
#endif()
#set(CMAKE_CXX_FLAGS_DEBUG "-rdynamic -g")

target_include_directories(${PROJECT_NAME}
	PRIVATE
		hrs/
		Renderer/
		LuaWay/)

target_link_libraries(${PROJECT_NAME}
	                    PRIVATE Hrs
						PRIVATE Renderer
						PRIVATE LuaWay
						#PRIVATE GeometryParser
						#PRIVATE WindowSubSystem
					)

