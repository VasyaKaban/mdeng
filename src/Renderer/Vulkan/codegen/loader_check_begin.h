#define FIRE_LAND_LOADER_CHECK_USE(LOADER) \
{ \
	const auto &fire_land_loader_use = LOADER; \

#define FIRE_LAND_LOADER_CHECK_FUNCTION(NAME) \
	if(!fire_land_loader_use.NAME) \
		return {#NAME};

#define FIRE_LAND_LOADER_CHECK_FUNCTION_IF(NAME) \
{ \
	constexpr auto fire_land_loader_check_function_name = #NAME; \
	FIRE_LAND_LOADER_CHECK_FUNCTION_ALT(NAME)

#define FIRE_LAND_LOADER_CHECK_FUNCTION_ALT(NAME) \
	if(!fire_land_loader_check_function_name.NAME) \

#define FIRE_LAND_LOADER_CHECK_FUNCTION_IF_SELECTOR(NAME, ...) \
{ \
	auto fire_land_loader_check_function_selector = __VA_ARGS__; \
	constexpr auto fire_land_loader_check_function_name = #NAME; \
	FIRE_LAND_LOADER_CHECK_FUNCTION_ALT_SELECTOR(NAME)

#define FIRE_LAND_LOADER_CHECK_FUNCTION_ALT_SELECTOR(NAME) \
	if(fire_land_loader_check_function_name.NAME) \
		fire_land_loader_check_function_selector(fire_land_loader_use.NAME); \
	else

#define FIRE_LAND_LOADER_CHECK_FUNCTION_ENDIF() \
	return {fire_land_loader_check_function_name}; \
}


#define FIRE_LAND_LOADER_CHECK_UNUSE() \
}
