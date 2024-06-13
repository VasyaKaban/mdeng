#pragma once

#include "../Vulkan/VulkanInclude.h"

namespace FireLand
{
	struct DeviceLoader
	{
		PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
		PFN_vkDestroyDevice vkDestroyDevice;

		DeviceLoader() noexcept;

		bool Init(VkDevice device, PFN_vkGetDeviceProcAddr instance_vkGetDeviceProcAddr) noexcept;
	};
};
