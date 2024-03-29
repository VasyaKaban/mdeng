#pragma once

#include "../BufferPool/BufferPool.hpp"

namespace FireLand
{
	struct IndexBufferCreator
	{
		static vk::DeviceSize GetBlockSizePower() noexcept;
		static vk::BufferUsageFlags GetBufferUsage() noexcept;
		static std::span<const MemoryPropertyOpFlags> GetAllocationVariants() noexcept;
		static NewPoolSizeCalculator GetNewPoolSizeCalculator() noexcept;
	};

	using IndexBuffer = BufferPool<IndexBufferCreator>;
};

