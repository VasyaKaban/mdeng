/**
 * @file
 *
 * Represents EmbeddedOperation
 */

#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include <functional>

namespace FireLand
{
	/**
	 * @brief The EmbeddedOperation using
	 *
	 * Used for making operation between transfer operations (like making a barrier)
	 */
	using EmbeddedOperation = std::function<void (const vk::CommandBuffer &)>;
};
