#pragma once

#include <array>
#include "ObjData.h"

namespace GeometryParser
{
	enum class NormalCalculationType
	{
		VertexCommon,
		ClockWise,
		CounterClockWise
	};

	enum class NormalCalculationSystemType
	{
		LeftHanded,
		RightHanded
	};

	std::array<float, 3> CalculateNormal(const ObjData &obj_data,
										 std::span<const std::size_t> indices,
										 NormalCalculationType type,
										 NormalCalculationSystemType system) noexcept;
};
