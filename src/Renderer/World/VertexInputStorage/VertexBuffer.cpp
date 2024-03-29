#include "VertexBuffer.h"

namespace FireLand
{
	vk::DeviceSize VertexBufferCreator::GetBlockSizePower() noexcept
	{
		return 1;
	}

	vk::BufferUsageFlags VertexBufferCreator::GetBufferUsage() noexcept
	{
		return vk::BufferUsageFlagBits::eVertexBuffer;
	}

	std::span<const MemoryPropertyOpFlags> VertexBufferCreator::GetAllocationVariants() noexcept
	{
		constexpr static MemoryPropertyOpFlags variants[] =
		{
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOp::Only, {}},
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOp::Any, {}},
			{{}, MemoryTypeSatisfyOp::Any, {}},

			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOp::Only,
			 AllocationFlags::AllowPlaceWithMixedResources},
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOp::Any,
			 AllocationFlags::AllowPlaceWithMixedResources},
			{{}, MemoryTypeSatisfyOp::Any, AllocationFlags::AllowPlaceWithMixedResources},
		};

		return variants;
	}

	NewPoolSizeCalculator VertexBufferCreator::GetNewPoolSizeCalculator() noexcept
	{
		return MemoryType::DefaultNewPoolSizeCalculator;
	}
};
