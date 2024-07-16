#pragma once

#include "basic_multikey_map.hpp"

namespace hrs
{
	template<typename V, type_instantiation<key> CKey, type_instantiation<key> ...CKeys>
	using multikey_map = basic_multikey_map<V, std::allocator<std::tuple<V, CKey, CKeys...>>, CKey, CKeys...>;
};
