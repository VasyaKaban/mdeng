#pragma once

#include <type_traits>

namespace hrs
{
	template<typename T, template <typename ...> typename D>
	struct is_type_instantiation : std::false_type {};

	template<typename ...Args, template <typename ...> typename D>
	struct is_type_instantiation<D<Args...>, D> : std::true_type {};

	template<typename T, template <typename ...> typename D>
	constexpr inline bool is_type_instantiation_v = is_type_instantiation<T, D>::value;

	template<typename T, template <typename ...> typename D>
	concept type_instantiation = is_type_instantiation_v<T, D>;

	template<typename T, template <auto ...> typename D>
	struct is_non_type_instantiation : std::false_type {};

	template<auto ...Args, template <auto ...> typename D>
	struct is_non_type_instantiation<D<Args...>, D> : std::true_type {};

	template<typename T, template <auto ...> typename D>
	constexpr inline bool is_non_type_instantiation_v = is_non_type_instantiation<T, D>::value;

	template<typename T, template <auto ...> typename D>
	concept non_type_instantiation = is_non_type_instantiation_v<T, D>;
};
