#include "Linerizer.h"
#include <utility>

namespace GeometryParser
{
	Linerizer::Linerizer(ObjData &_obj_data) noexcept
		: obj_data(&_obj_data),
		  index(0) {}

	void Linerizer::Reset(ObjData &_obj_data) noexcept
	{
		obj_data = &_obj_data;
		index = 0;
	}

	Linerizer::operator bool() const noexcept
	{
		return obj_data;
	}

	std::optional<std::array<std::size_t, 2>> Linerizer::Next() noexcept
	{
		if(index == obj_data->GetIndices().size())
			return {};

		std::array<std::size_t, 2> out;
		if(obj_data->GetSchema().face_count == 2)
		{
			out[0] = index;
			out[1] = index + 1;

			index += 2;
		}
		else
		{
			if((index + 1) % obj_data->GetSchema().face_count == 0)
			{
				out[0] = index;
				out[1] = (index / obj_data->GetSchema().face_count) * obj_data->GetSchema().face_count;
			}
			else
			{
				out[0] = index;
				out[1] = index + 1;
			}

			index++;
		}

		return out;
	}
};

