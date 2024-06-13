#include "InstanceLoader.h"

namespace FireLand
{
	InstanceLoader::InstanceLoader() noexcept
		: vkGetInstanceProcAddr(nullptr),
		  vkDestroyInstance(nullptr),
		  vkCreateDevice(nullptr),
		  vkGetDeviceProcAddr(nullptr) {}

	bool InstanceLoader::Init(VkInstance instance, PFN_vkGetInstanceProcAddr global_vkGetInstanceProcAddr) noexcept
	{
		vkGetInstanceProcAddr =
			reinterpret_cast<PFN_vkGetInstanceProcAddr>(global_vkGetInstanceProcAddr(instance, "vkGetInstanceProcAddr"));

		if(!vkGetInstanceProcAddr)
			return false;

		vkDestroyInstance
			= reinterpret_cast<PFN_vkDestroyInstance>(global_vkGetInstanceProcAddr(instance, "vkDestroyInstance"));

		vkCreateDevice
			= reinterpret_cast<PFN_vkCreateDevice>(global_vkGetInstanceProcAddr(instance, "vkCreateDevice"));

		vkGetDeviceProcAddr
			= reinterpret_cast<PFN_vkGetDeviceProcAddr>(global_vkGetInstanceProcAddr(instance, "vkGetDeviceProcAddr"));

		return true;
	}
};
