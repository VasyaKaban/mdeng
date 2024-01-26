#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include <functional>

namespace FireLand
{
	struct RenderInputs
	{
		vk::Extent2D resolution;
		bool is_resized;
		vk::PresentModeKHR present_mode;

		RenderInputs(const vk::Extent2D &_resolution = {},
					 bool _is_resized = {},
					 vk::PresentModeKHR _present_mode = {})
			: resolution(_resolution),
			  is_resized(_is_resized),
			  present_mode(_present_mode) {}

		~RenderInputs() = default;
		RenderInputs(const RenderInputs &) = default;
		RenderInputs(RenderInputs &&) = default;
		RenderInputs & operator=(const RenderInputs &) = default;
		RenderInputs & operator=(RenderInputs &&) = default;
	};
};
