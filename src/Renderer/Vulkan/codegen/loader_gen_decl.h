#define FIRE_LAND_DETAIL_CONCAT1(M_PREFIX, TYPE) M_PREFIX##TYPE
#define FIRE_LAND_DETAIL_CONCAT(M_PREFIX, TYPE) FIRE_LAND_DETAIL_CONCAT1(M_PREFIX, TYPE)

#define FIRE_LAND_LOADER_BEGIN_GLOBAL(NAME, INIT_NAME) \
class NAME \
{ \
private: \
	constexpr static std::size_t COUNTER_START = __COUNTER__ + 1; \
public: \
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr; \
	std::optional<std::size_t> INIT_NAME(PFN_vkGetInstanceProcAddr lib_vkGetInstanceProcAddr) noexcept;

#define FIRE_LAND_LOADER_FUNCTION_GLOBAL(NAME) \
	PFN_##NAME NAME = (void(__COUNTER__), nullptr);

#define FIRE_LAND_LOADER_END_GLOBAL() \
	constexpr static std::size_t COUNT = __COUNTER__ - COUNTER_START; \
};

#define FIRE_LAND_LOADER_BEGIN_INSTANCE(NAME, INIT_NAME) \
class NAME \
{ \
private: \
	constexpr static std::size_t COUNTER_START = __COUNTER__ + 1; \
public: \
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr; \
	std::optional<std::size_t> INIT_NAME(VkInstance instance, \
										 PFN_vkGetInstanceProcAddr global_vkGetInstanceProcAddr) noexcept;

#define FIRE_LAND_LOADER_FUNCTION_INSTANCE(NAME) \
FIRE_LAND_LOADER_FUNCTION_GLOBAL(NAME)

#define FIRE_LAND_LOADER_END_INSTANCE() \
FIRE_LAND_LOADER_END_GLOBAL()

#define FIRE_LAND_LOADER_BEGIN_DEVICE(NAME, INIT_NAME) \
class NAME \
{ \
private: \
	constexpr static std::size_t COUNTER_START = __COUNTER__ + 1; \
public: \
	PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = nullptr; \
	std::optional<std::size_t> INIT_NAME(VkDevice device, \
										 PFN_vkGetDeviceProcAddr instance_vkGetDeviceProcAddr) noexcept;

#define FIRE_LAND_LOADER_FUNCTION_DEVICE(NAME) \
	FIRE_LAND_LOADER_FUNCTION_GLOBAL(NAME)

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
