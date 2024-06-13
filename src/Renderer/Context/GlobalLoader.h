#pragma once

#include "../Vulkan/VulkanInclude.h"

namespace FireLand
{
	class VulkanLibrary;

	struct GlobalLoader
	{
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
		PFN_vkCreateInstance vkCreateInstance;
		PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
		PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
#ifdef VK_VERSION_1_1
		PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
#endif

		GlobalLoader() noexcept;

		bool Init(const VulkanLibrary &lib) noexcept;
	};
};
