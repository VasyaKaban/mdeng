cmake_minimum_required(VERSION 3.5)

project(mdeng LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src/hrs)
add_subdirectory(src/Renderer)
add_subdirectory(src/GeometryParser)

set(Sources
	src/main.cpp)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(${PROJECT_NAME} ${Sources})

set(Libs
	PRIVATE ${Vulkan_LIBRARIES}
	PRIVATE ${SDL2_LIBRARIES}
	PRIVATE Hrs
	PRIVATE Renderer
	PRIVATE GeometryParser)

target_link_libraries(${PROJECT_NAME} ${Libs})

