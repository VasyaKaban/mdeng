#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "../hrs/math/matrix.hpp"

namespace FireLand
{
	class Camera
	{
	public:
		virtual ~Camera() {}

		virtual const vk::Viewport & GetViewport() const noexcept = 0;
		virtual const vk::Rect2D & GetScissorsRect() const noexcept = 0;
		virtual hrs::math::glsl::std430::mat4x4 GetComputedMatrix() const noexcept = 0;
	};
};
