#define FIRE_LAND_IM_LOADER_BEGIN(NAME, INIT_NAME) \
class NAME \
{ \
private: \
	constexpr static std::size_t COUNTER_START = __COUNTER__ + 1; \
public: \
	std::optional<std::string_view> INIT_NAME(

#define FIRE_LAND_IM_LOADER_SOURCE(SRC, ...) \
SRC __VA_OPT__(, __VA_ARGS__)) noexcept;

#define FIRE_LAND_IM_LOADER_USE_SOURCE(NAME) /*noop*/

#define FIRE_LAND_IM_LOADER_UNUSE_SOURCE() /*noop*/

#define FIRE_LAND_IM_LOADER_FUNCTION(NAME) \
PFN_##NAME NAME = (void(__COUNTER__), nullptr);

#define FIRE_LAND_IM_LOADER_FUNCTION_IF(NAME) \
FIRE_LAND_IM_LOADER_FUNCTION(NAME)

#define FIRE_LAND_IM_LOADER_FUNCTION_ALT(NAME) /*noop*/

#define FIRE_LAND_IM_LOADER_FUNCTION_ENDIF() /*noop*/

#define FIRE_LAND_IM_LOADER_END() \
	constexpr static std::size_t COUNT = __COUNTER__ - COUNTER_START; \
};

#ifdef IM_LOADER_GEN_LIST
IM_LOADER_GEN_LIST()
#endif

#undef FIRE_LAND_IM_LOADER_BEGIN
#undef FIRE_LAND_IM_LOADER_SOURCE
#undef FIRE_LAND_IM_LOADER_USE_SOURCE
#undef FIRE_LAND_IM_LOADER_UNUSE_SOURCE
#undef FIRE_LAND_IM_LOADER_FUNCTION
#undef FIRE_LAND_IM_LOADER_FUNCTION_IF
#undef FIRE_LAND_IM_LOADER_FUNCTION_ALT
#undef FIRE_LAND_IM_LOADER_FUNCTION_ENDIF
#undef FIRE_LAND_IM_LOADER_END
