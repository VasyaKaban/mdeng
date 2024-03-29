#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include "Scene/Scene.h"
#include "../../hrs/math/matrix.hpp"
#include "../../hrs/math/matrix_view.hpp"

namespace FireLand
{
	struct RenderInputs
	{
		hrs::math::glsl::std430::mat4x4 view_point_matrix;
		vk::Viewport view_point_viewport;
		vk::Rect2D view_point_scissors;

		vk::Extent2D new_swapchain_resolution;
		vk::PresentModeKHR new_present_mode;

		const Scene *scene;

		RenderInputs(hrs::math::matrix_view<float, 4, 4> _view_point_matrix,
					 const vk::Viewport &_view_point_viewport,
					 const vk::Rect2D &_view_point_scissors,
					 const vk::Extent2D &_new_swapchain_resolution,
					 vk::PresentModeKHR _new_present_mode,
					 const Scene *_scene) noexcept
			: view_point_matrix(_view_point_matrix),
			  view_point_viewport(_view_point_viewport),
			  view_point_scissors(_view_point_scissors),
			  new_swapchain_resolution(_new_swapchain_resolution),
			  new_present_mode(_new_present_mode),
			  scene(_scene) {}


		~RenderInputs() = default;
		RenderInputs(const RenderInputs &) = default;
		RenderInputs(RenderInputs &&) = default;
		RenderInputs & operator=(const RenderInputs &) = default;
		RenderInputs & operator=(RenderInputs &&) = default;
	};
};
