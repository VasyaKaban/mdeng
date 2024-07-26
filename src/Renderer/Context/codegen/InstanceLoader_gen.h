//#ifndef NDEBUG
	#define INSTANCE_LOADER_DEBUG_FUNCTIONS() \
		/*debug messenger*/ \
		FIRE_LAND_LOADER_FUNCTION(vkCreateDebugUtilsMessengerEXT) \
		FIRE_LAND_LOADER_FUNCTION(vkDestroyDebugUtilsMessengerEXT)
//#else
//	#define INSTANCE_LOADER_DEBUG_FUNCTIONS()
//#endif


#define LOADER_GEN_TYPE() INSTANCE
#define LOADER_GEN_LIST() \
	FIRE_LAND_LOADER_BEGIN(InstanceLoader, Init) \
		/*instance*/ \
		FIRE_LAND_LOADER_FUNCTION(vkCreateInstance) \
		FIRE_LAND_LOADER_FUNCTION(vkDestroyInstance) \
		/*physical device*/ \
		FIRE_LAND_LOADER_FUNCTION(vkEnumeratePhysicalDevices) \
		FIRE_LAND_LOADER_FUNCTION(vkGetPhysicalDeviceProperties) \
		/*logical device*/ \
		FIRE_LAND_LOADER_FUNCTION(vkCreateDevice) \
		FIRE_LAND_LOADER_FUNCTION(vkDestroyDevice) \
		/*surface*/ \
		FIRE_LAND_LOADER_FUNCTION(vkDestroySurfaceKHR) \
		/*debug*/ \
		INSTANCE_LOADER_DEBUG_FUNCTIONS() \
	FIRE_LAND_LOADER_END()

#include "../../Vulkan/VulkanInclude.h"
