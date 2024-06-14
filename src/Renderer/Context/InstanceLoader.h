#pragma once

#include "../Vulkan/VulkanInclude.h"
#include "../Vulkan/VulkanLoaderGenBegin.h"

namespace FireLand
{
	struct InstanceLoader
	{
		FIRE_LAND_DECL_VK_FUNCTION(GetInstanceProcAddr)
		FIRE_LAND_DECL_VK_FUNCTION(DestroyInstance)
		FIRE_LAND_DECL_VK_FUNCTION(CreateDevice)
		FIRE_LAND_DECL_VK_FUNCTION(DestroyDevice)
		FIRE_LAND_DECL_VK_FUNCTION(EnumeratePhysicalDevices)
		FIRE_LAND_DECL_VK_FUNCTION(DestroySurfaceKHR)
		FIRE_LAND_DECL_VK_FUNCTION(GetDeviceProcAddr)

		bool Init(VkInstance handle, PFN_vkGetInstanceProcAddr global_vkGetInstanceProcAddr) noexcept;
	};
};

#include "../Vulkan/VulkanLoaderGenEnd.h"
