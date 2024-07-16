#pragma once

#include <type_traits>
#include "class_meta.hpp"
#include "enum_meta.hpp"

namespace hrs
{
	template<typename T>
	struct reflexpr_info
	{
		using type = void;
	};

	template<typename C>
		requires std::is_class_v<C> || std::is_union_v<C>
	struct reflexpr_info<C>
	{
		using type = class_meta<C>;
	};

	template<typename E>
		requires std::is_enum_v<E>
	struct reflexpr_info<E>
	{
		using type = enum_meta<E>;
	};

	template<typename T>
	using reflexpr = reflexpr_info<T>::type;
};
