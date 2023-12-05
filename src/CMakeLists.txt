cmake_minimum_required(VERSION 3.5)

project(mdeng LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Sources
	main.cpp
	hrs/expected.hpp
	hrs/block.hpp
	hrs/free_block_allocator.hpp
	hrs/debug.hpp
	hrs/scoped_call.hpp
	Vulkan/VulkanInclude.hpp
	Allocator/Memory.hpp
	Allocator/Allocator.hpp
	Allocator/Buffer.hpp
	Allocator/MemoryType.hpp
	Allocator/ConditionalAllocate.hpp
	Transfer/Region.hpp
	Transfer/Transfer.hpp
	Transfer/BufferRegion.hpp
	Transfer/EmbeddedOperation.hpp
	Transfer/ImageRegion.hpp
	VulkanUtils/BufferWithProperty.hpp
	FreeBlockBuffer/FreeBlockBuffer.hpp
	UniformInstanceIndexBufferHub/RemoveRequest.hpp
	UniformInstanceIndexBufferHub/AcquireRequest.hpp
	UniformInstanceIndexBufferHub/UpdateRequest.hpp
	UniformInstanceIndexBufferHub/UniformInstanceIndexAllocator.hpp
	UniformInstanceIndexBufferHub/IndexPool.hpp
	UniformInstanceIndexBufferHub/UniformInstanceIndexBufferHub.hpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../out/debug/")

find_package(Vulkan REQUIRED)

add_executable(${PROJECT_NAME} ${Sources})

set(Libs ${Vulkan_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${Libs})

