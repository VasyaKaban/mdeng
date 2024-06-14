#include "InstanceLoader.h"
#include "../Vulkan/VulkanLoaderGenBegin.h"

namespace FireLand
{
	bool InstanceLoader::Init(VkInstance handle,
							  PFN_vkGetInstanceProcAddr global_vkGetInstanceProcAddr) noexcept
	{
		if(!global_vkGetInstanceProcAddr || handle == VK_NULL_HANDLE)
			return false;

		FIRE_LAND_RESOLVE_VK_FUNCTION_COND(global_vkGetInstanceProcAddr, handle, GetInstanceProcAddr,false)
		FIRE_LAND_RESOLVE_VK_FUNCTION_COND(GetInstanceProcAddr, handle, GetInstanceProcAddr, false)
		FIRE_LAND_RESOLVE_VK_FUNCTION_COND(GetInstanceProcAddr, handle, DestroyInstance, false)
		FIRE_LAND_RESOLVE_VK_FUNCTION_COND(GetInstanceProcAddr, handle, CreateDevice, false)
		FIRE_LAND_RESOLVE_VK_FUNCTION_COND(GetInstanceProcAddr, handle, DestroyDevice, false)

		FIRE_LAND_RESOLVE_VK_FUNCTION(GetInstanceProcAddr, handle, EnumeratePhysicalDevices)
		FIRE_LAND_RESOLVE_VK_FUNCTION(GetInstanceProcAddr, handle, DestroySurfaceKHR)
		FIRE_LAND_RESOLVE_VK_FUNCTION(GetInstanceProcAddr, handle, GetDeviceProcAddr)

		return true;
	}
};

#include "../Vulkan/VulkanLoaderGenEnd.h"
