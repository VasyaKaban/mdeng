/*#pragma once

#include <utility>

namespace hrs
{
	template<typename T>
	concept castable_to_bool = requires(T &&t)
	{
		static_cast<bool>(std::forward<T>(t));
	};

	template<typename T>
	concept equal_comparable_single = requires(T &&t)
	{
		std::forward<T>(t) == std::forward<T>(t);
	};

	template<typename T, typename U>
	concept equal_comparable = requires(T &&t, U &&u)
	{
		std::forward<T>(t) == std::forward<U>(u);
	};
};

#define HRS_RETURN_PLAIN_TRUE(comp) \
	static_assert(hrs::castable_to_bool<decltype(comp)>); \
	if(comp) { return; }

#define HRS_RETURN_PLAIN_FALSE(comp) \
	static_assert(hrs::castable_to_bool<decltype(comp)>); \
	if(!comp) { return; }

#define HRS_RETURN_TARGET_TRUE(comp) \
	static_assert(hrs::castable_to_bool<decltype(comp)>); \
	if(comp) { return comp; }

#define HRS_RETURN_TARGET_FALSE(comp) \
	static_assert(hrs::castable_to_bool<decltype(comp)>); \
	if(!comp) { return comp; }

#define HRS_RETURN_TARGET_TRUE_ALT(comp, alt) \
	static_assert(hrs::castable_to_bool<decltype(comp)>); \
	if(comp) { return alt; }

#define HRS_RETURN_TARGET_FALSE_ALT(comp, alt) \
	static_assert(hrs::castable_to_bool<decltype(comp)>); \
	if(!comp) { return alt; }

#define HRS_RETURN_TARGET_TRUE_WRAP(comp, WRAPPER) \
	static_assert(std::constructible_from<WRAPPER, decltype(comp)>); \
	static_assert(hrs::castable_to_bool<decltype(comp)>)); \
	if(comp) { return WRAPPER(comp); }

#define HRS_RETURN_TARGET_FALSE_WRAP(comp, WRAPPER, braces_init) \
	static_assert(std::constructible_from<WRAPPER, decltype(comp)>); \
	static_assert(hrs::castable_to_bool<decltype(comp)>)); \
	if(!comp) { return WRAPPER(comp); }

#define HRS_RETURN_PLAIN_CMP_TRUE(comp1, comp2) \
	static_assert(hrs::equal_comparable<decltype(comp1), decltype(comp2)>); \
	if(comp1 == comp2) { return; }

#define HRS_RETURN_PLAIN_CMP_FALSE(comp1, comp2) \
	static_assert(hrs::equal_comparable<decltype(comp1), decltype(comp2)>); \
	if(comp1 == comp2) { return };

#define HRS_RETURN_TARGET1_CMP_TRUE(comp1, comp2) \
	static_assert(hrs::equal_comparable<decltype(comp1), decltype(comp2)>); \
	if(comp1 == comp2) { return comp1; }

#define HRS_RETURN_TARGET1_CMP_FALSE(comp1, comp2) \
	static_assert(hrs::equal_comparable<decltype(comp1), decltype(comp2)>); \
	if(comp1 != comp2) { return comp1; }

#define HRS_RETURN_TARGET2_CMP_TRUE(comp1, comp2) \
	static_assert(hrs::equal_comparable<decltype(comp1), decltype(comp2)>); \
	if(comp1 == comp2) { return comp2; }

#define HRS_RETURN_TARGET2_CMP_FALSE(comp1, comp2) \
	static_assert(hrs::equal_comparable<decltype(comp1), decltype(comp2)>); \
	if(comp1 != comp2) { return comp2; }

#define HRS_RETURN_TARGET1_CMP_TRUE_WRAP(comp1, comp2, WRAPPER) \
	static_assert(std::constructible_from<WRAPPER, decltype(comp1)>); \
	static_assert(hrs::equal_comparable<decltype(comp1), decltype(comp2)>); \
	if(comp1 == comp2) { return WRAPPER(comp1); }

#define HRS_RETURN_TARGET1_CMP_FALSE_WRAP(comp1, comp2, WRAPPER) \
static_assert(std::constructible_from<WRAPPER, decltype(comp1)>); \
	static_assert(hrs::equal_comparable<decltype(comp1), decltype(comp2)>); \
	if(comp1 != comp2) { return WRAPPER(comp1); }

#define HRS_RETURN_TARGET2_CMP_TRUE_WRAP(comp1, comp2, WRAPPER) \
	static_assert(std::constructible_from<WRAPPER, decltype(comp1)>); \
	static_assert(hrs::equal_comparable<decltype(comp1), decltype(comp2)>); \
	if(comp1 == comp2) { return WRAPPER(comp2); }

#define HRS_RETURN_TARGET2_CMP_FALSE_WRAP(comp1, comp2, WRAPPER) \
	static_assert(std::constructible_from<WRAPPER, decltype(comp1)>); \
	static_assert(hrs::equal_comparable<decltype(comp1), decltype(comp2)>); \
	if(comp1 != comp2) { return WRAPPER(comp2); }

*/
