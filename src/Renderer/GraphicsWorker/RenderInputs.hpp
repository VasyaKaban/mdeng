#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include "../../Camera/ComputedCamera.h"
#include "../../Scene/Scene.h"

namespace FireLand
{
	struct RenderInputs
	{
		ComputedCamera view_point;
		const Scene *scene;
		vk::Extent2D resolution;
		bool is_resized;
		vk::PresentModeKHR present_mode;

		RenderInputs(const Camera *_view_point,
					 const Scene *_scene,
					 const vk::Extent2D &_resolution,
					 bool _is_resized,
					 vk::PresentModeKHR _present_mode)
			: scene(_scene),
			  view_point(_view_point),
			  resolution(_resolution),
			  is_resized(_is_resized),
			  present_mode(_present_mode) {}

		~RenderInputs() = default;
		RenderInputs(const RenderInputs &) = default;
		RenderInputs(RenderInputs &&) = default;
		RenderInputs & operator=(const RenderInputs &) = default;
		RenderInputs & operator=(RenderInputs &&) = default;
	};
};
