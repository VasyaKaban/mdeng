#define FIRE_LAND_DETAIL_CONCAT1(M_PREFIX, TYPE) M_PREFIX##TYPE
#define FIRE_LAND_DETAIL_CONCAT(M_PREFIX, TYPE) FIRE_LAND_DETAIL_CONCAT1(M_PREFIX, TYPE)

#define FIRE_LAND_LOADER_BEGIN_GLOBAL(NAME, INIT_NAME) \
class NAME \
{ \
private: \
	constexpr static std::size_t COUNTER_START = __COUNTER__ + 1; \
public: \
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr; \
	LoaderInitResult INIT_NAME(PFN_vkGetInstanceProcAddr lib_vkGetInstanceProcAddr) noexcept;

#define FIRE_LAND_LOADER_BEGIN_INSTANCE(NAME, INIT_NAME) \
class NAME \
{ \
private: \
	constexpr static std::size_t COUNTER_START = __COUNTER__ + 1; \
public: \
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr; \
	LoaderInitResult INIT_NAME(VkInstance instance, \
							   PFN_vkGetInstanceProcAddr global_vkGetInstanceProcAddr) noexcept;

#define FIRE_LAND_LOADER_BEGIN_DEVICE(NAME, INIT_NAME) \
class NAME \
{ \
private: \
	constexpr static std::size_t COUNTER_START = __COUNTER__ + 1; \
public: \
	PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = nullptr; \
	LoaderInitResult INIT_NAME(VkDevice device, \
							   PFN_vkGetDeviceProcAddr instance_vkGetDeviceProcAddr) noexcept;

#define FIRE_LAND_LOADER_BEGIN_MIXED(NAME, INIT_NAME) \
class NAME \
{ \
private: \
	constexpr static std::size_t COUNTER_START = __COUNTER__ + 1; \
public: \
	LoaderInitResult INIT_NAME(VkInstance instance, \
							   PFN_vkGetInstanceProcAddr instance_vkGetInstanceProcAddr, \
							   VkDevice device, \
							   PFN_vkGetDeviceProcAddr device_vkGetDeviceProcAddr) noexcept;


#define FIRE_LAND_MIXED_LOADER_USE_INSTANCE_SOURCE() /*noop*/
#define FIRE_LAND_MIXED_LOADER_USE_DEVICE_SOURCE() /*noop*/
#define FIRE_LAND_MIXED_LOADER_UNUSE_SOURCE() /*noop*/

#define FIRE_LAND_LOADER_FUNCTION(NAME) \
	PFN_##NAME NAME = (void(__COUNTER__), nullptr);

#define FIRE_LAND_LOADER_REQUIRED_FUNCTION(NAME) \
FIRE_LAND_LOADER_FUNCTION(NAME)

#define FIRE_LAND_LOADER_FUNCTION_IF(NAME) \
FIRE_LAND_LOADER_FUNCTION(NAME)

#define FIRE_LAND_LOADER_FUNCTION_ALT(NAME) /*noop*/

#define FIRE_LAND_LOADER_FUNCTION_ENDIF() /*noop*/

#define FIRE_LAND_LOADER_END() \
};

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
