#pragma once

#include "../BufferPool/BufferPool.hpp"

namespace FireLand
{
	struct VertexBufferCreator
	{
		static vk::DeviceSize GetBlockSizePower() noexcept;
		static vk::BufferUsageFlags GetBufferUsage() noexcept;
		static std::span<const MemoryPropertyOpFlags> GetAllocationVariants() noexcept;
		static NewPoolSizeCalculator GetNewPoolSizeCalculator() noexcept;
	};

	using VertexBuffer = BufferPool<VertexBufferCreator>;
};
