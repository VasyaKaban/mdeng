#include "Camera.h"

namespace FireLand
{
	struct VPCamera : public Camera
	{
		hrs::math::glsl::mat3x3 view_rotate;
		hrs::math::glsl::vec3 view_offset;
		hrs::math::glsl::vec4 projection_values;//x-mul, y-mul, z-mul, z-add
		vk::Viewport viewport;
		vk::Rect2D scissors_rect;

		VPCamera(const hrs::math::glsl::mat3x3 &_view_rotate = {},
				 const hrs::math::glsl::vec3 &_view_offset = {},
				 const hrs::math::glsl::vec4 &_projection_values = {},
				 const vk::Viewport &_viewport = {},
				 const vk::Rect2D &_scissors_rect = {});
		~VPCamera() = default;
		VPCamera(const VPCamera &) = default;
		VPCamera(VPCamera &&) = default;
		VPCamera & operator=(const VPCamera &) = default;
		VPCamera & operator=(VPCamera &&) = default;

		void SetProjection(float fov, float aspect, float near, float far) noexcept;

		virtual vk::Viewport GetViewport() noexcept override;
		virtual vk::Rect2D GetScissorsRect() noexcept override;
		virtual hrs::math::glsl::std430::mat4x4 GetMatrix() noexcept override;
	};
};
