#include "ComputedCamera.h"
#include "../hrs/debug.hpp"

namespace FireLand
{
	ComputedCamera::ComputedCamera(const Camera *_camera) noexcept
		: camera(_camera)
	{
		hrs::assert_true_debug(camera, "Camera pointer points to null!");

		computed_matrix = camera->GetComputedMatrix();
	}

	const hrs::math::glsl::std430::mat4x4 & ComputedCamera::GetComputedMatrix() const noexcept
	{
		return computed_matrix;
	}

	void ComputedCamera::RecomputeMatrix() noexcept
	{
		computed_matrix = camera->GetComputedMatrix();
	}

	const Camera * ComputedCamera::GetCamera() const noexcept
	{
		return camera;
	}
};
