cmake_minimum_required(VERSION 3.5)

project(mdeng LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Sources
	main.cpp
	hrs/expected.hpp
	hrs/block.hpp
	hrs/mem_req.hpp
	hrs/debug.hpp
	hrs/scoped_call.hpp
	hrs/flags.hpp
	hrs/non_creatable.hpp
	hrs/iterator_for_each.hpp
	hrs/error.hpp
	hrs/instantiation.hpp
	hrs/free_block_chain_base.hpp
	hrs/sized_free_block_chain.hpp
	hrs/unsized_free_block_chain.hpp
	hrs/hint_cast_object.hpp

	hrs/math/math_common.hpp
	hrs/math/vector_common.hpp
	hrs/math/vector.hpp
	hrs/math/vector_view.hpp
	hrs/math/matrix_common.hpp
	hrs/math/matrix.hpp
	hrs/math/matrix_view.hpp
	hrs/math/quaternion.hpp

	Vulkan/VulkanInclude.hpp
	Vulkan/VulkanUtils.hpp
	Vulkan/VulkanFormatUtils.hpp

	Renderer/Allocator/Memory.h
	Renderer/Allocator/Memory.cpp
	Renderer/Allocator/MemoryPool.h
	Renderer/Allocator/MemoryPool.cpp
	Renderer/Allocator/MemoryPoolLists.h
	Renderer/Allocator/MemoryPoolLists.cpp
	Renderer/Allocator/MemoryType.h
	Renderer/Allocator/MemoryType.cpp
	Renderer/Allocator/Allocator.h
	Renderer/Allocator/Allocator.cpp
	Renderer/Allocator/AllocateFromMany.hpp
	Renderer/Allocator/BoundedResourceSize.hpp

	Renderer/Context/Context.h
	Renderer/Context/Context.cpp
	Renderer/Context/DeviceWorker.h
	Renderer/Context/PhysicalDevice.h
	Renderer/Context/PhysicalDevice.cpp
	Renderer/Context/Device.h
	Renderer/Context/Device.cpp
	Renderer/Context/Surface.h
	Renderer/Context/Surface.cpp

	Renderer/TransferChannel/TransferChannel.h
	Renderer/TransferChannel/TransferChannel.cpp
	Renderer/TransferChannel/TransferBufferOp.hpp
	Renderer/TransferChannel/TransferImageOp.hpp
	Renderer/TransferChannel/Data.h
	Renderer/TransferChannel/Data.cpp

	Renderer/World/RenderWorld/Stateful.h
	Renderer/World/RenderWorld/PlainStateful.h
	Renderer/World/RenderWorld/PlainStateful.cpp
	Renderer/World/RenderWorld/RenderWorld.h
	Renderer/World/RenderWorld/RenderWorld.cpp
	Renderer/World/RenderWorld/RenderPass.h
	Renderer/World/RenderWorld/RenderPass.cpp
	Renderer/World/RenderWorld/Mesh.h
	Renderer/World/RenderWorld/Shader.h
	Renderer/World/RenderWorld/Shader.cpp
	Renderer/World/RenderWorld/Material.h
	Renderer/World/RenderWorld/MaterialGroup.h
	Renderer/World/RenderWorld/MaterialGroup.cpp
	Renderer/World/RenderWorld/RenderGroup.h
	Renderer/World/RenderWorld/RenderGroup.cpp


	Renderer/World/ObjectWorld/ObjectWorld.h
	Renderer/World/ObjectWorld/ObjectWorld.cpp
	Renderer/World/ObjectWorld/Object.h
	Renderer/World/ObjectWorld/Object.cpp
	Renderer/World/ObjectWorld/ObjectInstance.h
	Renderer/World/ObjectWorld/ObjectInstance.cpp
	Renderer/World/ObjectWorld/ObjectMeshBinding.h
	Renderer/World/ObjectWorld/ObjectMeshBinding.cpp


	Renderer/ImageStorage/Image.h
	Renderer/ImageStorage/Image.cpp
	Renderer/ImageStorage/ImageView.h
	Renderer/ImageStorage/ImageView.cpp
	Renderer/ImageStorage/ImageStorage.h
	Renderer/ImageStorage/ImageStorage.cpp

	Renderer/DescriptorStorage/DescriptorPool.h
	Renderer/DescriptorStorage/DescriptorPool.cpp
	Renderer/DescriptorStorage/DescriptorStorage.h
	Renderer/DescriptorStorage/DescriptorStorage.cpp

	Renderer/DataIndexStorage/IndexPool.h
	Renderer/DataIndexStorage/IndexPool.cpp
	Renderer/DataIndexStorage/DataIndexStorage.h
	Renderer/DataIndexStorage/DataIndexStorage.cpp

	Renderer/DataBuffer/DataQueue.h
	Renderer/DataBuffer/DataQueue.cpp
	Renderer/DataBuffer/DataBuffer.h
	Renderer/DataBuffer/DataBuffer.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../out/debug/")

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(${PROJECT_NAME} ${Sources})

set(Libs ${Vulkan_LIBRARIES} ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${Libs})

