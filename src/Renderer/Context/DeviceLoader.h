#pragma once

#include "../Vulkan/VulkanInclude.h"
#include "../Vulkan/VulkanLoaderGenBegin.h"

namespace FireLand
{
	struct DeviceLoader
	{
		FIRE_LAND_DECL_VK_FUNCTION(GetDeviceProcAddr)
		FIRE_LAND_DECL_VK_FUNCTION(DestroyDevice)

		bool Init(VkDevice handle, PFN_vkGetDeviceProcAddr instance_vkGetDeviceProcAddr) noexcept;
	};
};

#include "../Vulkan/VulkanLoaderGenEnd.h"
