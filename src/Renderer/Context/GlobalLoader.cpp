#include "GlobalLoader.h"
#include "../Vulkan/VulkanLibrary.h"

namespace FireLand
{
	GlobalLoader::GlobalLoader() noexcept
		: vkGetInstanceProcAddr(nullptr),
		  vkCreateInstance(nullptr),
		  vkEnumerateInstanceExtensionProperties(nullptr),
		  vkEnumerateInstanceLayerProperties(nullptr)
#ifdef VK_VERSION_1_1
		 ,vkEnumerateInstanceVersion(nullptr) {}
#endif

	bool GlobalLoader::Init(const VulkanLibrary &lib) noexcept
	{
		vkGetInstanceProcAddr
			= reinterpret_cast<PFN_vkGetInstanceProcAddr>(lib.GetProcAddress("vkGetInstanceProcAddr"));

		if(!vkGetInstanceProcAddr)
			return false;

		vkCreateInstance =
			reinterpret_cast<PFN_vkCreateInstance>(vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstace"));

		vkEnumerateInstanceExtensionProperties =
			reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties"));

		vkEnumerateInstanceLayerProperties =
			reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceLayerProperties"));

#ifdef VK_VERSION_1_1
		vkEnumerateInstanceVersion =
			reinterpret_cast<PFN_vkEnumerateInstanceVersion>(vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"));
#endif

		return true;
	}
};
