cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Renderer STATIC)

target_sources(
	Renderer
	    PRIVATE
		Vulkan/VulkanInclude.h
		Vulkan/LoaderInitResult.h
		Vulkan/LoaderInitResult.cpp
		Vulkan/InitResult.h
		Vulkan/InitResult.cpp
		Vulkan/VkResultMeta.hpp
		Vulkan/QueueFamilyIndex.h
		Vulkan/codegen/loader_gen_decl.h
		Vulkan/codegen/loader_gen_def.h
		Vulkan/codegen/loader_check_begin.h
		Vulkan/codegen/loader_check_end.h
)

target_sources(
	Renderer
	    PRIVATE
		Context/Context.h
		Context/Context.cpp
		Context/GlobalLoader.h
		Context/GlobalLoader.cpp
		Context/InstanceLoader.h
		Context/InstanceLoader.cpp
		Context/DeviceUtilizer.h
		Context/DeviceLoader.h
		Context/DeviceLoader.cpp

		Context/codegen/GlobalLoader_gen.h
		Context/codegen/InstanceLoader_gen.h
		Context/codegen/DeviceLoader_gen.h
)

target_sources(
	Renderer
	    PRIVATE
		Allocator/AllocatorResult.h
		Allocator/Memory.h
		Allocator/Memory.cpp
		Allocator/MemoryPool.h
		Allocator/MemoryPool.cpp
		Allocator/MemoryPoolLists.h
		Allocator/MemoryPoolLists.cpp
		Allocator/MemoryType.h
		Allocator/MemoryType.cpp
		Allocator/Allocator.h
		Allocator/Allocator.cpp
		Allocator/Bounded.h
		Allocator/Bounded.cpp
		Allocator/BoundedSize.h
		Allocator/BoundedSize.cpp
)

target_sources(
	Renderer
	    PRIVATE
		TransferChannel/BoundedBufferSizeFillness.h
		TransferChannel/BoundedBufferSizeFillness.cpp
		TransferChannel/TransferBufferOp.h
		TransferChannel/TransferImageOp.h
		TransferChannel/TransferChannel.h
		TransferChannel/TransferChannel.cpp
)

target_sources(
	Renderer
	    PRIVATE
		DescriptorStorage/DescriptorPool.h
		DescriptorStorage/DescriptorPool.cpp
		DescriptorStorage/DescriptorStorage.h
		DescriptorStorage/DescriptorStorage.cpp
)

set(WORLD_FOLDER_SOURCES
	World/RenderWorld/Stateful.h
	World/RenderWorld/PlainStateful.h
	World/RenderWorld/PlainStateful.cpp
	World/RenderWorld/RenderWorld.h
	World/RenderWorld/RenderWorld.cpp
	World/RenderWorld/RenderPass.h
	World/RenderWorld/RenderPass.cpp
	World/RenderWorld/Mesh.h
	World/RenderWorld/Shader.h
	World/RenderWorld/Shader.cpp
	World/RenderWorld/Material.h
	World/RenderWorld/MaterialGroup.h
	World/RenderWorld/MaterialGroup.cpp
	World/RenderWorld/RenderGroup.h
	World/RenderWorld/RenderGroup.cpp

	World/ObjectWorld/ObjectWorld.h
	World/ObjectWorld/ObjectWorld.cpp
	World/ObjectWorld/Object.h
	World/ObjectWorld/Object.cpp
	World/ObjectWorld/ObjectInstance.h
	World/ObjectWorld/ObjectInstance.cpp
	World/ObjectWorld/ObjectMeshBinding.h
	World/ObjectWorld/ObjectMeshBinding.cpp)

set(DATA_INDEX_STORAGE_FOLDER_SOURCES
	DataIndexStorage/IndexPool.h
	DataIndexStorage/IndexPool.cpp
	DataIndexStorage/DataIndexStorage.h
	DataIndexStorage/DataIndexStorage.cpp)

#make version for static objects and dynamic
set(DATA_BUFFER_FOLDER_SOURCES
	DataBuffer/DataQueue.h
	DataBuffer/DataQueue.cpp
	DataBuffer/DataBuffer.h
	DataBuffer/DataBuffer.cpp)

set(SWAPCHAIN_FOLDER_SOURCES
	Swapchain/Swapchain.h
	Swapchain/Swapchain.cpp)

link_directories(../hrs)
target_link_libraries(Renderer PRIVATE Hrs)
target_include_directories(Renderer PUBLIC ../)
