#pragma once

#include "MemoryPool.hpp"
#include "../hrs/error.hpp"

namespace FireLand
{
	enum class AllocationResult
	{
		Success,
		NoSatisfiedMemoryTypes,
		NotEnoughSpaceInPool,
		NoSatisfiedMemoryPools,
	};

	constexpr auto AllocationResultToString(AllocationResult result) noexcept
	{
		switch(result)
		{
			case AllocationResult::Success:
				return "Success";
				break;
			case AllocationResult::NoSatisfiedMemoryTypes:
				return "NoSatisfiedMemoryTypes";
				break;
			case AllocationResult::NotEnoughSpaceInPool:
				return "NotEnoughSpaceInPool";
				break;
			case AllocationResult::NoSatisfiedMemoryPools:
				return "NoSatisfiedMemoryPools";
				break;
		}

		return "";
	}

	using AllocationError = hrs::error<vk::Result, AllocationResult>;

	enum class AllocationFlags
	{
		AllocateSeparateMemory = 1 << 0,//allocate separate memory for resource
		AllocateWithMixedResources = 1 << 1,//allocate memory with mixed resource types
		AllowCreationWithMixedResources = 1 << 2,//allow resource creation with mixed resource memory
		BindToWholeMemory = 1 << 3,//bind to whole allocated memory(works with AllocateSeparateMemory flag)
		MapAllocatedMemory = 1 << 4,//map allocated memory
		CreateOnlyOnExistedPools = 1 << 5//allow creation only on existed pools(conflicts with AllocateSeparateMemory)
	};

	struct BoundedBuffer
	{
		vk::Buffer buffer;
		std::uint32_t memory_type_index;
		std::list<MemoryPool>::iterator memory_pool_handle;
		hrs::block<vk::DeviceSize> block;

		BoundedBuffer(vk::Buffer _buffer = {},
					  std::uint32_t _memory_type_index = {},
					  std::list<MemoryPool>::iterator _memory_pool_handle = {},
					  const hrs::block<vk::DeviceSize> &_block = {})
			: buffer(_buffer),
			  memory_type_index(_memory_type_index),
			  memory_pool_handle(_memory_pool_handle),
			  block(_block) {}

		constexpr bool IsCreated() const noexcept
		{
			return buffer;
		}
	};

	struct BoundedImage
	{
		vk::Image image;
		std::uint32_t memory_type_index;
		std::list<MemoryPool>::iterator memory_pool_handle;
		hrs::block<vk::DeviceSize> block;

		BoundedImage(vk::Image _image = {},
					 std::uint32_t _memory_type_index = {},
					 std::list<MemoryPool>::iterator _memory_pool_handle = {},
					 const hrs::block<vk::DeviceSize> &_block = {})
			: image(_image),
			  memory_type_index(_memory_type_index),
			  memory_pool_handle(_memory_pool_handle),
			  block(_block) {}

		constexpr bool IsCreated() const noexcept
		{
			return image;
		}
	};
};
