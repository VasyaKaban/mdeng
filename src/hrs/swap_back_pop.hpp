#pragma once

#include <vector>

namespace hrs
{
	template<typename T>
	constexpr void swap_back_pop(std::vector<T> &vec, typename std::vector<T>::iterator it)
	{
		if(vec.empty())
			return;

		if(it != std::prev(vec.end()))
		{
			std::swap(*it, vec.back());
			vec.pop_back();
		}
	}
};
