#define LOADER_GEN_LIST() \
	FIRE_LAND_LOADER_BEGIN_GLOBAL(GlobalLoader, Init) \
		FIRE_LAND_LOADER_REQUIRED_FUNCTION(vkCreateInstance) \
		FIRE_LAND_LOADER_FUNCTION(vkEnumerateInstanceVersion) \
		FIRE_LAND_LOADER_REQUIRED_FUNCTION(vkEnumerateInstanceExtensionProperties) \
		FIRE_LAND_LOADER_REQUIRED_FUNCTION(vkEnumerateInstanceLayerProperties) \
	FIRE_LAND_LOADER_END()
