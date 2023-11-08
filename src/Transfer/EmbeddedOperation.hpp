#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include <functional>

namespace FireLand
{
	using EmbeddedOperation = std::function<auto (const vk::CommandBuffer &) -> void>;
};
