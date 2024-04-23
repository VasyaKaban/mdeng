#include "Triangulator.h"
#include <utility>

namespace GeometryParser
{
	Triangulator::Triangulator(ObjData &_obj_data) noexcept
		: obj_data(&_obj_data),
		  index(0) {}

	void Triangulator::Reset(ObjData &_obj_data) noexcept
	{
		obj_data = &_obj_data;
		index = 0;
	}

	Triangulator::operator bool() const noexcept
	{
		return obj_data;
	}

	std::optional<std::array<std::size_t, 3>> Triangulator::Next() noexcept
	{
		if(index == obj_data->GetIndices().size())
			return {};

		std::array<std::size_t, 3> out;
		if(obj_data->GetSchema().face_count == 3)
		{
			out[0] = index;
			out[1] = index + 1;
			out[2] = index + 2;

			index += 3;
		}
		else
		{
			out[0] = index;
			out[1] = index + 1;

			if(index % obj_data->GetSchema().face_count == 0)
			{
				out[2] = index + 2;
				index += 2;
			}
			else
			{
				out[2] = (index / obj_data->GetSchema().face_count) * obj_data->GetSchema().face_count;
				if((index + 2) % obj_data->GetSchema().face_count == 0)
					index += 2;
				else
					index++;
			}
		}

		return out;
	}
};
