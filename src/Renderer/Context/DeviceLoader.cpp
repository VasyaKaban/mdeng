#include "DeviceLoader.h"
#include "../Vulkan/VulkanLoaderGenBegin.h"

namespace FireLand
{
	bool DeviceLoader::Init(VkDevice handle, PFN_vkGetDeviceProcAddr instance_vkGetDeviceProcAddr) noexcept
	{
		if(handle == VK_NULL_HANDLE || !instance_vkGetDeviceProcAddr)
			return false;

		FIRE_LAND_RESOLVE_VK_FUNCTION_COND(instance_vkGetDeviceProcAddr, handle, GetDeviceProcAddr, false)
		FIRE_LAND_RESOLVE_VK_FUNCTION_COND(GetDeviceProcAddr, handle, GetDeviceProcAddr, false)
		FIRE_LAND_RESOLVE_VK_FUNCTION_COND(GetDeviceProcAddr, handle, DestroyDevice, false)

		return true;
	}
};

#include "../Vulkan/VulkanLoaderGenEnd.h"
