cmake_minimum_required(VERSION 3.5)

project(mdeng LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Sources
	main.cpp
	hrs/expected.hpp
	hrs/block.hpp
	hrs/mem_req.hpp
	#hrs/free_block_allocator.hpp
	hrs/debug.hpp
	hrs/scoped_call.hpp
	hrs/flags.hpp
	hrs/non_creatable.hpp
	hrs/iterator_for_each.hpp
	#hrs/variadic.hpp
	hrs/error.hpp
	hrs/instantiation.hpp
	#hrs/unexpected_result.hpp
	hrs/free_block_chain_base.hpp
	hrs/sized_free_block_chain.hpp
	hrs/unsized_free_block_chain.hpp
	hrs/hint_cast_object.hpp
	#hrs/early_return.hpp
	#hrs/tuple_utils.hpp
	#hrs/variadic.hpp
	#hrs/function_traits.hpp
	#hrs/callback.hpp
	Vulkan/VulkanInclude.hpp
	Vulkan/VulkanUtils.hpp
	Vulkan/VulkanFormatUtils.hpp

	#Allocator/Allocator.hpp
	#Allocator/MemoryType.hpp
	#Allocator/MemoryPool.hpp
	#Allocator/Memory.hpp
	#Allocator/AllocatorTypes.hpp
	#Allocator/UnexpectedAllocationResult.hpp


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

	Renderer/World/BufferPool/BufferPool.hpp
	Renderer/World/VertexInputStorage/VertexBuffer.h
	Renderer/World/VertexInputStorage/VertexBuffer.cpp
	Renderer/World/VertexInputStorage/IndexBuffer.h
	Renderer/World/VertexInputStorage/IndexBuffer.cpp
	Renderer/World/VertexInputStorage/VertexInputStorage.h
	Renderer/World/VertexInputStorage/VertexInputStorage.cpp

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

	Renderer/World/World.h
	Renderer/World/World.cpp
	Renderer/World/StaticObject/ReifiedObject.h
	Renderer/World/StaticObject/Mesh.h
	Renderer/World/StaticObject/Mesh.cpp
	Renderer/World/StaticObject/Object.h
	Renderer/World/StaticObject/Object.cpp
	Renderer/World/StaticObject/ObjectInstance.h
	Renderer/World/StaticObject/ObjectInstance.cpp
	Renderer/World/StaticObject/MeshMaterialBinding.h
	Renderer/World/StaticObject/MeshMaterialBinding.cpp

	Renderer/World/StaticObject/ReifiedObjects/PositionTextureNormalTangentObject.h
	Renderer/World/StaticObject/ReifiedObjects/PositionTextureNormalTangentObject.cpp

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

	#Renderer/RenderingWorker/RenderingWorker.h
	#Renderer/RenderingWorker/RenderingWorker.cpp
	#Renderer/RenderingWorker/GBuffer/GBuffer.h
	#Renderer/RenderingWorker/GBuffer/GBuffer.cpp
	#Renderer/RenderingWorker/PostProcessImages/PostProcessImages.h
	#Renderer/RenderingWorker/PostProcessImages/PostProcessImages.cpp
	#Renderer/RenderingWorker/PerFrameResources.h
	#Renderer/RenderingWorker/PerFrameResources.cpp

	#Renderer/RenderingWorker/Scene/Scene.h
	#Renderer/RenderingWorker/RenderInputs.hpp

	#Renderer/RenderingWorker/Shader/Shader.h
	#Renderer/RenderingWorker/Shader/ShaderInfoNode.hpp

	#Renderer/RenderingWorker/RenderPass/DefferedPass/DefferedPassShader.h
	#Renderer/RenderingWorker/RenderPass/DefferedPass/DefferedPass.h
	#Renderer/RenderingWorker/RenderPass/DefferedPass/DefferedPass.cpp


	#Renderer/GraphicsWorker/GraphicsWorker.h
	#Renderer/GraphicsWorker/GraphicsWorker.cpp
	#Renderer/GraphicsWorker/PerFrameResources.h
	#Renderer/GraphicsWorker/PerFrameResources.cpp
	#Renderer/GraphicsWorker/RenderInputs.hpp
	#Renderer/GraphicsWorker/Swapchain/Swapchain.h
	#Renderer/GraphicsWorker/Swapchain/Swapchain.cpp
	#Renderer/GraphicsWorker/RenderpassesOutputImage.h
	#Renderer/GraphicsWorker/RenderpassesOutputImage.cpp
	#Renderer/GraphicsWorker/FlaggedSwapchain.h
	#Renderer/GraphicsWorker/FlaggedSwapchain.cpp

	#Renderer/GraphicsWorker/RenderPass/DefferedPass/DefferedPass.h
	#Renderer/GraphicsWorker/RenderPass/DefferedPass/DefferedPass.cpp
	#Renderer/GraphicsWorker/RenderPass/DefferedPass/GBuffer.h
	#Renderer/GraphicsWorker/RenderPass/DefferedPass/GBuffer.cpp
	#Renderer/GraphicsWorker/RenderPass/DefferedPass/DefferedPassShader.h

	#Renderer/GraphicsWorker/RenderPass/DefferedPass/Shaders/TestDefferedShader.h
	#Renderer/GraphicsWorker/RenderPass/DefferedPass/Shaders/TestDefferedShader.cpp
	#Renderer/GraphicsWorker/RenderPass/DefferedPass/Shaders/TestEvalShader.h
	#Renderer/GraphicsWorker/RenderPass/DefferedPass/Shaders/TestEvalShader.cpp

	#Scene/Scene.h
	#Renderer/Shader/Shader.h
	#Renderer/Shader/ShaderInfoNode.hpp

	#Camera/Camera.h
	#Camera/ComputedCamera.h
	#Camera/ComputedCamera.cpp
	#Camera/VPCamera.h
	#Camera/VPCamera.cpp

	#TestScene/TestScene.h
	#TestScene/TestScene.cpp

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

