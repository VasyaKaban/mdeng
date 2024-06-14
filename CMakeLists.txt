cmake_minimum_required(VERSION 3.28)

project(mdeng LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src/hrs)
add_subdirectory(src/Renderer)
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

add_executable(${PROJECT_NAME} src/main.cpp)

#add_dependencies(${PROJECT_NAME} generated_loader)

set(CMAKE_CXX_FLAGS_DEBUG "-rdynamic -g")

target_link_libraries(${PROJECT_NAME}
	                    PRIVATE Hrs
						PRIVATE Renderer
						#PRIVATE GeometryParser
						#PRIVATE WindowSubSystem
					)

