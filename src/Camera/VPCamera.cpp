#include "VPCamera.h"

namespace FireLand
{
	VPCamera::VPCamera(const hrs::math::glsl::mat3x3 &_view_rotate,
					   const hrs::math::glsl::vec3 &_view_offset,
					   const hrs::math::glsl::vec4 &_projection_values,
					   const vk::Viewport &_viewport,
					   const vk::Rect2D &_scissors_rect)
		: view_rotate(_view_rotate), view_offset(_view_offset),
		  projection_values(_projection_values), viewport(_viewport),
		  scissors_rect(_scissors_rect) {}

	void VPCamera::SetProjection(float fov, float aspect, float near, float far) noexcept
	{
		float half_tan = std::tan(fov / 2);
		float top = half_tan * near;
		float right = aspect * top;
		projection_values[0] = 1.0f / right;//x mul
		projection_values[1] = 1.0f / top;//y mul
		projection_values[2] = far / (far - near);//z mul
		projection_values[3] = -(near * far) / (far - near);//z add
	}

	const vk::Viewport & VPCamera::GetViewport() const noexcept
	{
		return viewport;
	}

	const vk::Rect2D & VPCamera::GetScissorsRect() const noexcept
	{
		return scissors_rect;
	}

	hrs::math::glsl::std430::mat4x4 VPCamera::GetComputedMatrix() const noexcept
	{
		auto projection_mat = hrs::math::glsl::std430::mat4x4::identity(1.0f);
		projection_mat[0][0] = projection_values[0];
		projection_mat[1][1] = projection_values[1];
		projection_mat[2][2] = projection_values[2];
		projection_mat[3][2] = projection_values[3];
		projection_mat[2][3] = 1.0f;

		hrs::math::glsl::std430::mat4x4 view_mat(view_rotate);
		view_mat[3][0] = view_offset[0];
		view_mat[3][1] = view_offset[1];
		view_mat[3][2] = view_offset[2];

		return view_mat * projection_mat;
	}
};
