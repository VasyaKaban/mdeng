#pragma once

#include "Camera.h"

namespace FireLand
{
	class ComputedCamera
	{
	public:
		ComputedCamera(const Camera *_camera) noexcept;
		~ComputedCamera() = default;
		ComputedCamera(const ComputedCamera &) = default;
		ComputedCamera(ComputedCamera &&) = default;
		ComputedCamera & operator=(const ComputedCamera &) = default;
		ComputedCamera & operator=(ComputedCamera &&) = default;

		const hrs::math::glsl::std430::mat4x4 & GetComputedMatrix() const noexcept;
		void RecomputeMatrix() noexcept;
		const Camera * GetCamera() const noexcept;

	private:
		const Camera *camera;
		hrs::math::glsl::std430::mat4x4 computed_matrix;
	};
};
