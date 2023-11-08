#pragma once

#include "../Vulkan/VulkanInclude.hpp"

namespace FireLand
{
	struct MemoryType
	{
		vk::MemoryType memory_type;
		uint32_t index;

		constexpr MemoryType(vk::MemoryType _memory_type = {},
							 uint32_t _index = 0)
			: memory_type(_memory_type), index(_index) {}

		auto operator==(const MemoryType &) const noexcept -> bool = default;

		constexpr auto IsSatisfyAny(vk::MemoryPropertyFlags desired) const noexcept -> bool
		{
			return (memory_type.propertyFlags & desired) == desired;
		}

		constexpr auto IsSatisfyOnly(vk::MemoryPropertyFlags desired) const noexcept -> bool
		{
			return memory_type == desired;
		}

		constexpr auto IsSatisfy(const vk::MemoryRequirements &reqs) const noexcept -> bool
		{
			return (reqs.memoryTypeBits & (1 << index));
		}

		constexpr auto IsMappable() const noexcept -> bool
		{
			return static_cast<bool>((memory_type.propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible));
		}
	};
}
