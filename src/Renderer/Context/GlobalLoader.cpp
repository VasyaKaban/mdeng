#include "GlobalLoader.h"
#include "../Vulkan/VulkanLoaderGenBegin.h"

namespace FireLand
{
	bool GlobalLoader::Init(PFN_vkGetInstanceProcAddr global_vkGetInstanceProcAddr) noexcept
	{
		GetInstanceProcAddr = global_vkGetInstanceProcAddr;
		if(!GetInstanceProcAddr)
			return false;

		FIRE_LAND_RESOLVE_VK_FUNCTION_COND(GetInstanceProcAddr, VK_NULL_HANDLE, CreateInstance, false)

		FIRE_LAND_RESOLVE_VK_FUNCTION(GetInstanceProcAddr, VK_NULL_HANDLE, EnumerateInstanceExtensionProperties)
		FIRE_LAND_RESOLVE_VK_FUNCTION(GetInstanceProcAddr, VK_NULL_HANDLE, EnumerateInstanceLayerProperties)
		FIRE_LAND_RESOLVE_VK_FUNCTION(GetInstanceProcAddr, VK_NULL_HANDLE, EnumerateInstanceVersion)

		return true;
	}
};

#include "../Vulkan/VulkanLoaderGenEnd.h"

