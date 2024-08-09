//#ifndef NDEBUG
	#define INSTANCE_LOADER_DEBUG_FUNCTIONS() \
		/*debug messenger*/ \
		FIRE_LAND_LOADER_FUNCTION(vkCreateDebugUtilsMessengerEXT) \
		FIRE_LAND_LOADER_FUNCTION(vkDestroyDebugUtilsMessengerEXT)
//#else
//	#define INSTANCE_LOADER_DEBUG_FUNCTIONS()
//#endif


#define LOADER_GEN_LIST() \
	FIRE_LAND_LOADER_BEGIN_INSTANCE(InstanceLoader, Init) \
		/*instance*/ \
		FIRE_LAND_LOADER_REQUIRED_FUNCTION(vkDestroyInstance) \
		/*physical device*/ \
		FIRE_LAND_LOADER_REQUIRED_FUNCTION(vkEnumeratePhysicalDevices) \
		FIRE_LAND_LOADER_REQUIRED_FUNCTION(vkGetPhysicalDeviceProperties) \
		FIRE_LAND_LOADER_REQUIRED_FUNCTION(vkGetPhysicalDeviceMemoryProperties) \
		FIRE_LAND_LOADER_REQUIRED_FUNCTION(vkGetPhysicalDeviceImageFormatProperties) \
		FIRE_LAND_LOADER_REQUIRED_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties) \
		/*logical device*/ \
		FIRE_LAND_LOADER_REQUIRED_FUNCTION(vkGetDeviceProcAddr) \
		FIRE_LAND_LOADER_REQUIRED_FUNCTION(vkCreateDevice) \
		FIRE_LAND_LOADER_REQUIRED_FUNCTION(vkDestroyDevice) \
		/*surface*/ \
		FIRE_LAND_LOADER_FUNCTION(vkDestroySurfaceKHR) \
		/*debug*/ \
		INSTANCE_LOADER_DEBUG_FUNCTIONS() \
	FIRE_LAND_LOADER_END()

#include "../../Vulkan/VulkanInclude.h"
