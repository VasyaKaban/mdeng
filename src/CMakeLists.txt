cmake_minimum_required(VERSION 3.5)

project(mdeng LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Sources
	main.cpp
	hrs/expected.hpp
	hrs/block.hpp
	#hrs/free_block_allocator.hpp
	hrs/debug.hpp
	hrs/scoped_call.hpp
	hrs/relaxed_free_blocks_chain.hpp
	hrs/flags.hpp
	hrs/non_creatable.hpp
	hrs/iterator_for_each.hpp
	#hrs/variadic.hpp
	hrs/error.hpp
	hrs/instantiation.hpp
	hrs/unexpected_result.hpp
	#hrs/tuple_utils.hpp
	#hrs/variadic.hpp
	#hrs/function_traits.hpp
	#hrs/callback.hpp
	Vulkan/VulkanInclude.hpp
	Vulkan/UnexpectedVkResult.hpp
	Vulkan/VulkanUtils.hpp
	Vulkan/VulkanFormatUtils.hpp

	Allocator/Allocator.hpp
	Allocator/MemoryType.hpp
	Allocator/MemoryPool.hpp
	Allocator/Memory.hpp
	Allocator/AllocatorTypes.hpp
	Allocator/UnexpectedAllocationResult.hpp


	#Allocator/Memory.hpp
	#Allocator/Allocator.hpp
	#Allocator/Buffer.hpp
	#Allocator/MemoryType.hpp
	#Allocator/ConditionalAllocate.hpp
	#Transfer/Region.hpp
	#Transfer/Transfer.hpp
	#Transfer/BufferRegion.hpp
	#Transfer/EmbeddedOperation.hpp
	#Transfer/ImageRegion.hpp
	#Transfer/Data.hpp
	#VulkanUtils/BufferWithProperty.hpp
	#FreeBlockBuffer/FreeBlockBuffer.hpp
	#Scene/UniformInstanceIndexBufferHub/RemoveRequest.hpp
	#Scene/UniformInstanceIndexBufferHub/AcquireRequest.hpp
	#Scene/UniformInstanceIndexBufferHub/UpdateRequest.hpp
	#Scene/UniformInstanceIndexBufferHub/UniformInstanceIndexAllocator.hpp
	#Scene/UniformInstanceIndexBufferHub/IndexPool.hpp
	#Scene/UniformInstanceIndexBufferHub/UniformInstanceIndexBufferHub.hpp

	hrs/math/math_common.hpp
	hrs/math/vector_common.hpp
	hrs/math/vector.hpp
	hrs/math/vector_view.hpp
	hrs/math/matrix_common.hpp
	hrs/math/matrix.hpp
	hrs/math/matrix_view.hpp
	hrs/math/quaternion.hpp

	Renderer/Context/Context.h
	Renderer/Context/Context.cpp
	Renderer/Context/DeviceWorker.h
	Renderer/Context/PhysicalDevice.h
	Renderer/Context/PhysicalDevice.cpp
	Renderer/Context/Device.h
	Renderer/Context/Device.cpp
	Renderer/Context/Surface.h
	Renderer/Context/Surface.cpp

	Renderer/GraphicsWorker/GraphicsWorker.h
	Renderer/GraphicsWorker/GraphicsWorker.cpp
	Renderer/GraphicsWorker/PerFrameResources.h
	Renderer/GraphicsWorker/PerFrameResources.cpp
	Renderer/GraphicsWorker/RenderInputs.hpp
	Renderer/GraphicsWorker/Swapchain/Swapchain.h
	Renderer/GraphicsWorker/Swapchain/Swapchain.cpp
	Renderer/GraphicsWorker/RenderpassesOutputImage.h
	Renderer/GraphicsWorker/RenderpassesOutputImage.cpp
	Renderer/GraphicsWorker/FlaggedSwapchain.h
	Renderer/GraphicsWorker/FlaggedSwapchain.cpp

	Renderer/GraphicsWorker/RenderPass/DefferedPass/DefferedPass.h
	Renderer/GraphicsWorker/RenderPass/DefferedPass/DefferedPass.cpp
	Renderer/GraphicsWorker/RenderPass/DefferedPass/GBuffer.h
	Renderer/GraphicsWorker/RenderPass/DefferedPass/GBuffer.cpp
	Renderer/GraphicsWorker/RenderPass/DefferedPass/DefferedPassShader.h

	Renderer/GraphicsWorker/RenderPass/DefferedPass/Shaders/TestDefferedShader.h
	Renderer/GraphicsWorker/RenderPass/DefferedPass/Shaders/TestDefferedShader.cpp
	Renderer/GraphicsWorker/RenderPass/DefferedPass/Shaders/TestEvalShader.h
	Renderer/GraphicsWorker/RenderPass/DefferedPass/Shaders/TestEvalShader.cpp

	Scene/Scene.h
	Renderer/Shader/Shader.h
	Renderer/Shader/ShaderInfoNode.hpp

	Camera/Camera.h
	Camera/ComputedCamera.h
	Camera/ComputedCamera.cpp
	Camera/VPCamera.h
	Camera/VPCamera.cpp

	TestScene/TestScene.h
	TestScene/TestScene.cpp

	#ImagesTaker/ImagesMemoryTaker.hpp
	#ImagesTaker/ImagesMemoryTypeNode.hpp
	#ImagesTaker/ImagesFreeBlockBuffer.hpp
	#ImagesTaker/ImageBlock.hpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../out/debug/")

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(${PROJECT_NAME} ${Sources})

set(Libs ${Vulkan_LIBRARIES} ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${Libs})

