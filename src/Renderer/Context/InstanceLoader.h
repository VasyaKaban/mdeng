#pragma once

#include "../Vulkan/VulkanInclude.h"

namespace FireLand
{
	struct InstanceLoader
	{
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
		PFN_vkDestroyInstance vkDestroyInstance;
		PFN_vkCreateDevice vkCreateDevice;
		PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;

		InstanceLoader() noexcept;

		bool Init(VkInstance instance, PFN_vkGetInstanceProcAddr global_vkGetInstanceProcAddr) noexcept;
	};
};
