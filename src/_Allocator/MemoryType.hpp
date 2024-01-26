/**
 * @file
 *
 * Represents Vulkan allocator MemoryType
 */

#pragma once

#include "../Vulkan/VulkanInclude.hpp"

namespace FireLand
{
	/**
	 * @brief The MemoryType class
	 */
	struct MemoryType
	{
		vk::MemoryType memory_type;///<memory_type from VkPhysicalDeviceMemoryProperties.memoryTypes
		uint32_t index;///<index of memory type within VkPhysicalDeviceMemoryProperties

		constexpr MemoryType(vk::MemoryType _memory_type = {},
							 uint32_t _index = 0)
			: memory_type(_memory_type), index(_index) {}

		bool operator==(const MemoryType &) const noexcept = default;

		/**
		 * @brief IsSatisfyAny
		 * @param desired memory property flags that are desired to be
		 * @return true if propertyFlags is satisfy desired
		 *
		 * Checks whether all desired flags are set
		 */
		constexpr bool IsSatisfyAny(vk::MemoryPropertyFlags desired) const noexcept
		{
			return (memory_type.propertyFlags & desired) == desired;
		}

		/**
		 * @brief IsSatisfyOnly
		 * @param desired memory property flags that are desired to be
		 * @return true if propertyFlags is satisfy desired
		 *
		 * Checks whether only the desired flags are set
		 */
		constexpr bool IsSatisfyOnly(vk::MemoryPropertyFlags desired) const noexcept
		{
			return memory_type == desired;
		}

		/**
		 * @brief IsSatisfy
		 * @param reqs
		 * @return true if memory type is satisfy memory requirements
		 *
		 * Checks whether memory type satisfy memory requirements
		 */
		constexpr bool IsSatisfy(const vk::MemoryRequirements &reqs) const noexcept
		{
			return (reqs.memoryTypeBits & (1 << index));
		}

		/**
		 * @brief IsMappable
		 * @return true if memory type can be mapped
		 *
		 * Checks whether memory type can be mapped to host memory.
		 * Effectively checks whether propertyFlags have HostVisible flag set
		 */
		constexpr bool IsMappable() const noexcept
		{
			return static_cast<bool>((memory_type.propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible));
		}
	};
}
