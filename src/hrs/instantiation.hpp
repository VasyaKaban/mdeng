#pragma once

#include <type_traits>

namespace hrs
{
	template<typename T, template <typename ...> typename D>
	struct is_instantiation : std::false_type {};

	template<typename ...Args, template <typename ...> typename D>
	struct is_instantiation<D<Args...>, D> : std::true_type {};

	template<typename T, template <typename ...> typename D>
	constexpr inline bool is_instantiation_v = is_instantiation<T, D>::value;

	template<typename T, template <typename ...> typename D>
	concept instantiation = is_instantiation_v<T, D>;
};
