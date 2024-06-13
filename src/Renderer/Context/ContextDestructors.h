#pragma once

#include "../Vulkan/VulkanInclude.h"

namespace FireLand
{
	struct ContextDestructors
	{
		PFN_vkDestroyInstance vkDestroyInstance;
		PFN_vkDestroyDevice vkDestroyDevice;

		ContextDestructors() noexcept;

		void Init(PFN_vkGetInstanceProcAddr global_vkGetInstanceProcAddr) noexcept;
	};
};
