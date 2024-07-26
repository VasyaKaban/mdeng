#define FIRE_LAND_DETAIL_CONCAT1(M_PREFIX, TYPE) M_PREFIX##TYPE
#define FIRE_LAND_DETAIL_CONCAT(M_PREFIX, TYPE) FIRE_LAND_DETAIL_CONCAT1(M_PREFIX, TYPE)

#define FIRE_LAND_LOADER_BEGIN_GLOBAL(NAME, INIT_NAME) \
std::optional<std::size_t> NAME::INIT_NAME(PFN_vkGetInstanceProcAddr lib_vkGetInstanceProcAddr) noexcept \
{ \
	vkGetInstanceProcAddr = lib_vkGetInstanceProcAddr; \
	if(!vkGetInstanceProcAddr) \
		return {}; \
 \
	std::size_t loaded_count = 0;

#define FIRE_LAND_LOADER_FUNCTION_GLOBAL(NAME) \
	NAME = std::bit_cast<PFN_##NAME>(vkGetInstanceProcAddr(VK_NULL_HANDLE, #NAME)); \
	if(NAME) \
		loaded_count++;

#define FIRE_LAND_LOADER_END_GLOBAL() \
	return loaded_count; \
};

#define FIRE_LAND_LOADER_BEGIN_INSTANCE(NAME, INIT_NAME) \
std::optional<std::size_t> NAME::INIT_NAME(VkInstance instance, \
										   PFN_vkGetInstanceProcAddr global_vkGetInstanceProcAddr) noexcept \
{ \
	vkGetInstanceProcAddr = \
		std::bit_cast<PFN_vkGetInstanceProcAddr>(global_vkGetInstanceProcAddr(instance, "vkGetInstanceProcAddr")); \
	if(!vkGetInstanceProcAddr) \
		return {}; \
 \
	vkGetInstanceProcAddr = \
		std::bit_cast<PFN_vkGetInstanceProcAddr>(vkGetInstanceProcAddr(instance, "vkGetInstanceProcAddr")); \
	if(!vkGetInstanceProcAddr) \
		return {}; \
 \
	std::size_t loaded_count = 0;

#define FIRE_LAND_LOADER_FUNCTION_INSTANCE(NAME) \
	NAME = std::bit_cast<PFN_##NAME>(vkGetInstanceProcAddr(instance, #NAME)); \
	if(NAME) \
		loaded_count++;

#define FIRE_LAND_LOADER_END_INSTANCE() \
FIRE_LAND_LOADER_END_GLOBAL()

#define FIRE_LAND_LOADER_BEGIN_DEVICE(NAME, INIT_NAME) \
std::optional<std::size_t> NAME::INIT_NAME(VkDevice device, \
										   PFN_vkGetDeviceProcAddr instance_vkGetDeviceProcAddr) noexcept \
{ \
	vkGetDeviceProcAddr = \
		std::bit_cast<PFN_vkGetDeviceProcAddr>(instance_vkGetDeviceProcAddr(device, "vkGetDeviceProcAddr")); \
	if(!vkGetDeviceProcAddr) \
		return {}; \
 \
	vkGetDeviceProcAddr = \
		std::bit_cast<PFN_vkGetDeviceProcAddr>(vkGetDeviceProcAddr(device, "vkGetDeviceProcAddr")); \
	if(!vkGetDeviceProcAddr) \
		return {}; \
 \
	std::size_t loaded_count = 0;

#define FIRE_LAND_LOADER_FUNCTION_DEVICE(NAME) \
	NAME = std::bit_cast<PFN_##NAME>(vkGetDeviceProcAddr(device, #NAME)); \
	if(NAME) \
		loaded_count++;

#define FIRE_LAND_LOADER_END_DEVICE() \
FIRE_LAND_LOADER_END_GLOBAL()

#define FIRE_LAND_LOADER_BEGIN(NAME, INIT_NAME) \
FIRE_LAND_DETAIL_CONCAT(FIRE_LAND_LOADER_BEGIN_, LOADER_GEN_TYPE())(NAME, INIT_NAME)

#define FIRE_LAND_LOADER_FUNCTION(NAME) \
FIRE_LAND_DETAIL_CONCAT(FIRE_LAND_LOADER_FUNCTION_, LOADER_GEN_TYPE())(NAME)

#define FIRE_LAND_LOADER_END() \
FIRE_LAND_DETAIL_CONCAT(FIRE_LAND_LOADER_END_, LOADER_GEN_TYPE())()

#ifdef LOADER_GEN_LIST
LOADER_GEN_LIST()
#endif

#undef FIRE_LAND_DETAIL_CONCAT1
#undef FIRE_LAND_DETAIL_CONCAT

#undef FIRE_LAND_LOADER_BEGIN
#undef FIRE_LAND_LOADER_FUNCTION
#undef FIRE_LAND_LOADER_END

#undef FIRE_LAND_LOADER_BEGIN_GLOBAL
#undef FIRE_LAND_LOADER_FUNCTION_GLOBAL
#undef FIRE_LAND_LOADER_END_GLOBAL

#undef FIRE_LAND_LOADER_BEGIN_INSTANCE
#undef FIRE_LAND_LOADER_FUNCTION_INSTANCE
#undef FIRE_LAND_LOADER_END_INSTANCE

#undef FIRE_LAND_LOADER_BEGIN_DEVICE
#undef FIRE_LAND_LOADER_FUNCTION_DEVICE
#undef FIRE_LAND_LOADER_END_DEVICE
