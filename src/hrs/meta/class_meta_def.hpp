#define HRS_ALLOW_REFL_PRIVATE_ACCESS(NAME) friend class hrs::class_meta<NAME>;

#define HRS_REFL_BEGIN(NAME, ...) \
namespace hrs \
{ \
	template<> \
	struct class_meta<NAME> : detail::class_meta_base<NAME, \
													  detail::retrieve_namespace_and_name<#NAME>().second, \
													  meta_attributes<__VA_ARGS__>> \
	{

#define HRS_REFL_END() \
		static_assert(std::is_class_v<refl_class> || (std::is_union_v<refl_class> && parents::COUNT == 0), \
					  "Reflectable object must be either object or union without parents!"); \
	}; \
};

#define HRS_REFL_MEMBER_FIELDS_BEGIN() \
	using member_fields = hrs::variadic \
	<

#define HRS_REFL_MEMBER_FIELDS_END() \
	>; \

#define HRS_REFL_MEMBER_FIELD(NAME, ...) \
	member_field<&refl_class::NAME, #NAME __VA_OPT__(,) __VA_ARGS__>


#define HRS_REFL_USINGS_BEGIN() \
	using using_fields = hrs::variadic \
	<

#define HRS_REFL_USINGS_END() \
	>;

#define HRS_REFL_USING_FIELD(NAME, ...) \
	using_field<refl_class::NAME, #NAME, refl_class __VA_OPT__(,) __VA_ARGS__>

#define HRS_REFL_STATIC_FIELDS_BEGIN() \
	using static_fields = hrs::variadic \
	<

#define HRS_REFL_STATIC_FIELDS_END() \
	>;

#define HRS_REFL_PARENT_CLASSES(...) \
	using parents = hrs::variadic<__VA_ARGS__>;

#define HRS_REFL_STATIC_FIELD(NAME, ...) \
	static_field<&refl_class::NAME, #NAME, refl_class __VA_OPT__(,) __VA_ARGS__>
