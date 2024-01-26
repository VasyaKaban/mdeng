#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "../hrs/math/matrix.hpp"

namespace FireLand
{
	class Camera
	{
	public:
		virtual ~Camera() {}

		virtual vk::Viewport GetViewport() noexcept = 0;
		virtual vk::Rect2D GetScissorsRect() noexcept = 0;
		virtual hrs::math::glsl::std430::mat4x4 GetMatrix() noexcept = 0;
	};
};
