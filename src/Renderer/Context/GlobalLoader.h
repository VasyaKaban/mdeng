#pragma once

#include <type_traits>
#include "../Vulkan/VulkanInclude.h"
#include "../Vulkan/VulkanLoaderGenBegin.h"

namespace FireLand
{
	struct GlobalLoader
	{
		FIRE_LAND_DECL_VK_FUNCTION(GetInstanceProcAddr)
		FIRE_LAND_DECL_VK_FUNCTION(CreateInstance)
		FIRE_LAND_DECL_VK_FUNCTION(EnumerateInstanceExtensionProperties)
		FIRE_LAND_DECL_VK_FUNCTION(EnumerateInstanceLayerProperties)
		FIRE_LAND_DECL_VK_FUNCTION(EnumerateInstanceVersion)

		bool Init(PFN_vkGetInstanceProcAddr global_vkGetInstanceProcAddr) noexcept;
	};
};

#include "../Vulkan/VulkanLoaderGenEnd.h"
