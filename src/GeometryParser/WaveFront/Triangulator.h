#pragma once

#include "ObjData.h"

namespace GeometryParser
{
	class Triangulator
	{
	public:
		constexpr static std::size_t COMPONENTS = 3;

		Triangulator(ObjData &_obj_data) noexcept;
		~Triangulator() = default;
		Triangulator(const Triangulator &) = default;
		Triangulator(Triangulator &&) noexcept = default;
		Triangulator & operator=(const Triangulator &) = default;
		Triangulator & operator=(Triangulator &&) noexcept = default;

		void Reset(ObjData &_obj_data) noexcept;

		explicit operator bool() const noexcept;

		std::optional<std::array<std::size_t, 3>> Next() noexcept;

	private:
		ObjData *obj_data;
		std::size_t index;
	};
};
