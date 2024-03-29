#include "IndexBuffer.h"

namespace FireLand
{
	vk::DeviceSize IndexBufferCreator::GetBlockSizePower() noexcept
	{
		return sizeof(std::uint32_t);
	}

	vk::BufferUsageFlags IndexBufferCreator::GetBufferUsage() noexcept
	{
		return vk::BufferUsageFlagBits::eIndexBuffer;
	}

	std::span<const MemoryPropertyOpFlags> IndexBufferCreator::GetAllocationVariants() noexcept
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

	NewPoolSizeCalculator IndexBufferCreator::GetNewPoolSizeCalculator() noexcept
	{
		return MemoryType::DefaultNewPoolSizeCalculator;
	}
};
