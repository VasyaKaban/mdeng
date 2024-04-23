#include "NormalProcessing.h"
#include <utility>
#include <cmath>

namespace GeometryParser
{
	std::array<float, 3> CalculateNormal(const ObjData &obj_data,
										 std::span<const std::size_t> indices,
										 NormalCalculationType type,
										 NormalCalculationSystemType system) noexcept
	{
		if(indices.empty())
			return {};

		std::array<float, 3> normal = {0, 0, 0};
		auto cross_normal = [&normal](std::span<const float> v0, std::span<const float> v1)
		{
			normal[0] = v0[1] * v1[2] - v0[2] * v1[1];
			normal[1] =  v0[2] * v1[0] - v0[0] * v1[2];
			normal[2] = v0[0] * v1[1] - v0[1] * v1[0];
		};

		switch(type)
		{
			case NormalCalculationType::VertexCommon:
				{
					if(obj_data.GetSchema().face_type == FaceType::Vertex ||
						obj_data.GetSchema().face_type == FaceType::VertexTextureCoordinates)
						return {};

					std::size_t normal_index =
						(obj_data.GetSchema().face_type == FaceType::VertexNormal ? 1 : 2);

					for(std::uint32_t index : indices)
					{
						auto [face_index_ptr, id] = obj_data.GetFaceIndexIdPair(index);
						std::size_t start_index = face_index_ptr[normal_index] - 1;
						for(int i = 0; i < 3; i++)
							normal[i] += obj_data.GetNormals()[start_index * 3 + i];
					}

					for(int i = 0; i < 3; i++)
						normal[i] /= indices.size();
				}
				break;
			case NormalCalculationType::ClockWise:
			case NormalCalculationType::CounterClockWise:
				{
					if(indices.size() < 3)
						return {};

					std::array<const float *, 3> vertices;
					for(int i = 0; i < 3; i++)
					{
						auto [face_index_ptr, id] = obj_data.GetFaceIndexIdPair(indices[i]);
						std::size_t index = face_index_ptr[0] - 1;
						vertices[i] =
							&obj_data.GetVertices()[index * obj_data.GetSchema().vertex_components];
					}

					std::array<float, 3> v20, v10;
					for(int i = 0; i < 3; i++)
					{
						v20[i] = vertices[2][i] - vertices[0][i];
						v10[i] = vertices[1][i] - vertices[0][i];
					}

					if(system == NormalCalculationSystemType::LeftHanded)
					{
						if(type == NormalCalculationType::ClockWise)
							cross_normal(v20, v10);
						else
							cross_normal(v10, v20);
					}
					else
					{
						if(type == NormalCalculationType::ClockWise)
							cross_normal(v10, v20);
						else
							cross_normal(v20, v10);
					}

					float len = 0;
					for(int i = 0; i < 3; i++)
						len += normal[i] * normal[i];

					len = std::sqrt(len);
					for(int i = 0; i < 3; i++)
						normal[i] /= len;
				}
				break;
		}

		return normal;
	}

};
