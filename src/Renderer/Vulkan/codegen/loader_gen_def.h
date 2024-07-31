#define FIRE_LAND_DETAIL_CONCAT1(M_PREFIX, TYPE) M_PREFIX##TYPE
#define FIRE_LAND_DETAIL_CONCAT(M_PREFIX, TYPE) FIRE_LAND_DETAIL_CONCAT1(M_PREFIX, TYPE)

#define FIRE_LAND_LOADER_BEGIN_GLOBAL(NAME, INIT_NAME) \
LoaderInitResult NAME::INIT_NAME(PFN_vkGetInstanceProcAddr lib_vkGetInstanceProcAddr) noexcept \
{ \
	vkGetInstanceProcAddr = lib_vkGetInstanceProcAddr; \
	if(!vkGetInstanceProcAddr) \
		return std::monostate{}; \
 \
	std::size_t loaded_count = 0; \
	VkInstance resolver = VK_NULL_HANDLE; \
	PFN_vkGetInstanceProcAddr resolve_function = vkGetInstanceProcAddr;

#define FIRE_LAND_LOADER_BEGIN_INSTANCE(NAME, INIT_NAME) \
LoaderInitResult NAME::INIT_NAME(VkInstance instance, \
								 PFN_vkGetInstanceProcAddr global_vkGetInstanceProcAddr) noexcept \
{ \
	vkGetInstanceProcAddr = \
	reinterpret_cast<PFN_vkGetInstanceProcAddr>(global_vkGetInstanceProcAddr(instance, "vkGetInstanceProcAddr")); \
	if(!vkGetInstanceProcAddr) \
		return std::monostate{}; \
 \
	vkGetInstanceProcAddr = \
	reinterpret_cast<PFN_vkGetInstanceProcAddr>(vkGetInstanceProcAddr(instance, "vkGetInstanceProcAddr")); \
	if(!vkGetInstanceProcAddr) \
		return std::monostate{}; \
\
	std::size_t loaded_count = 0; \
	VkInstance resolver = instance; \
	PFN_vkGetInstanceProcAddr resolve_function = vkGetInstanceProcAddr;

#define FIRE_LAND_LOADER_BEGIN_DEVICE(NAME, INIT_NAME) \
LoaderInitResult NAME::INIT_NAME(VkDevice device, \
								 PFN_vkGetDeviceProcAddr instance_vkGetDeviceProcAddr) noexcept \
{ \
	vkGetDeviceProcAddr = \
	reinterpret_cast<PFN_vkGetDeviceProcAddr>(instance_vkGetDeviceProcAddr(device, "vkGetDeviceProcAddr")); \
	if(!vkGetDeviceProcAddr) \
		return std::monostate{}; \
 \
	vkGetDeviceProcAddr = \
	reinterpret_cast<PFN_vkGetDeviceProcAddr>(vkGetDeviceProcAddr(device, "vkGetDeviceProcAddr")); \
	if(!vkGetDeviceProcAddr) \
		return std::monostate{}; \
 \
	std::size_t loaded_count = 0; \
	VkDevice resolver = device; \
	PFN_vkGetDeviceProcAddr resolve_function = vkGetDeviceProcAddr;

#define FIRE_LAND_LOADER_BEGIN_MIXED(NAME, INIT_NAME) \
LoaderInitResult NAME::INIT_NAME(VkInstance instance, \
								 PFN_vkGetInstanceProcAddr instance_vkGetInstanceProcAddr, \
								 VkDevice device, \
								 PFN_vkGetDeviceProcAddr device_vkGetDeviceProcAddr) noexcept \
{ \
	std::size_t loaded_count = 0;

#define FIRE_LAND_MIXED_LOADER_USE_INSTANCE_SOURCE() \
{ \
	VkInstance resolver = instance; \
	PFN_vkGetInstanceProcAddr resolve_function = instance_vkGetInstanceProcAddr;


#define FIRE_LAND_MIXED_LOADER_USE_DEVICE_SOURCE() \
{ \
	VkDevice resolver = device; \
	PFN_vkGetDeviceProcAddr resolve_function = device_vkGetDeviceProcAddr;

#define FIRE_LAND_MIXED_LOADER_UNUSE_SOURCE() \
}

#define FIRE_LAND_LOADER_FUNCTION(NAME) \
	NAME = reinterpret_cast<PFN_##NAME>(resolve_function(resolver, #NAME)); \
	if(NAME) \
		loaded_count++;

#define FIRE_LAND_LOADER_REQUIRED_FUNCTION(NAME) \
	NAME = reinterpret_cast<PFN_##NAME>(resolve_function(resolver, #NAME)); \
	if(NAME) \
		loaded_count++; \
	else \
		return #NAME;

#define FIRE_LAND_LOADER_FUNCTION_IF(NAME) \
{ \
	constexpr static auto required_condition_function_name = #NAME; \
	auto &required_function = this->NAME; \
	NAME = reinterpret_cast<PFN_##NAME>(resolve_function(resolver, #NAME)); \
	if(!required_function)


#define FIRE_LAND_LOADER_FUNCTION_ALT(NAME) \
		if(required_function = reinterpret_cast<PFN_##NAME>(resolve_function(resolver, #NAME)), !required_function)

#define FIRE_LAND_LOADER_FUNCTION_ENDIF() \
		return required_condition_function_name; \
	else \
		loaded_count++; \
}

#define FIRE_LAND_LOADER_END() \
	return loaded_count; \
}

#ifdef LOADER_GEN_LIST
LOADER_GEN_LIST()
#endif

#undef FIRE_LAND_DETAIL_CONCAT1
#undef FIRE_LAND_DETAIL_CONCAT

#undef FIRE_LAND_LOADER_BEGIN_GLOBAL
#undef FIRE_LAND_LOADER_BEGIN_INSTANCE
#undef FIRE_LAND_LOADER_BEGIN_DEVICE
#undef FIRE_LAND_LOADER_BEGIN_MIXED

#undef FIRE_LAND_MIXED_LOADER_USE_INSTANCE_SOURCE
#undef FIRE_LAND_MIXED_LOADER_USE_DEVICE_SOURCE
#undef FIRE_LAND_MIXED_LOADER_UNUSE_SOURCE

#undef FIRE_LAND_LOADER_FUNCTION
#undef FIRE_LAND_LOADER_REQUIRED_FUNCTION
#undef FIRE_LAND_LOADER_FUNCTION_IF
#undef FIRE_LAND_LOADER_FUNCTION_ALT
#undef FIRE_LAND_LOADER_FUNCTION_ENDIF

#undef FIRE_LAND_LOADER_END
