#define LOADER_GEN_TYPE() GLOBAL
#define LOADER_GEN_LIST() \
	FIRE_LAND_LOADER_BEGIN(GlobalLoader, Init) \
		FIRE_LAND_LOADER_FUNCTION(vkCreateInstance) \
		FIRE_LAND_LOADER_FUNCTION(vkEnumerateInstanceVersion) \
		FIRE_LAND_LOADER_FUNCTION(vkEnumerateInstanceExtensionProperties) \
		FIRE_LAND_LOADER_FUNCTION(vkEnumerateInstanceLayerProperties) \
	FIRE_LAND_LOADER_END()
