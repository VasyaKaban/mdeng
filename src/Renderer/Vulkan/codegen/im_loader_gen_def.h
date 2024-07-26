#define FIRE_LAND_IM_LOADER_BEGIN(NAME, INIT_NAME) \
std::optional<std::string_view> NAME::INIT_NAME(

#define FIRE_LAND_IM_LOADER_SOURCE(SRC, ...) \
	SRC __VA_OPT__(, __VA_ARGS__)) noexcept \
{

#define FIRE_LAND_IM_LOADER_USE_SOURCE(NAME) \
	{ \
		auto &use_source = NAME;

#define FIRE_LAND_IM_LOADER_UNUSE_SOURCE() \
	}

#define FIRE_LAND_IM_LOADER_FUNCTION(NAME) \
	this->NAME = use_source.NAME; \
	if(!this->NAME) \
		return #NAME;

#define FIRE_LAND_IM_LOADER_FUNCTION_IF(NAME) \
	{ \
		constexpr static auto store_name_str = #NAME; \
		auto &store_name = this->NAME; \
		store_name = use_source.NAME; \
		if(!store_name) \


#define FIRE_LAND_IM_LOADER_FUNCTION_ALT(NAME) \
		if(store_name = use_source.NAME, !store_name)

#define FIRE_LAND_IM_LOADER_FUNCTION_ENDIF() \
		return store_name_str; \
	}

#define FIRE_LAND_IM_LOADER_END() \
	return {}; \
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
