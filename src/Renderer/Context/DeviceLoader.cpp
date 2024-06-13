#include "DeviceLoader.h"

namespace FireLand
{
	DeviceLoader::DeviceLoader() noexcept
		: vkGetDeviceProcAddr(nullptr),
		  vkDestroyDevice(nullptr) {}

	bool DeviceLoader::Init(VkDevice device, PFN_vkGetDeviceProcAddr instance_vkGetDeviceProcAddr) noexcept
	{
		vkGetDeviceProcAddr
			= reinterpret_cast<PFN_vkGetDeviceProcAddr>(instance_vkGetDeviceProcAddr(device, "vkGetDeviceProcAddr"));

		if(!vkGetDeviceProcAddr)
			return false;

		vkDestroyDevice
			= reinterpret_cast<PFN_vkDestroyDevice>(instance_vkGetDeviceProcAddr(device, "vkDestroyDevice"));

		return true;
	}
};
