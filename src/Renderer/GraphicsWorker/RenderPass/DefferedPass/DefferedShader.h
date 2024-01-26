#pragma once

#include "../../../../Vulkan/VulkanInclude.hpp"

namespace FireLand
{
	class DefferedShader
	{
	public:
		virtual ~DefferedShader() {};
		virtual void NotifyResize(const vk::Extent2D &resolution) = 0;
		virtual void Render(vk::CommandBuffer command_buffer) = 0;
		virtual void SetRenderParameters() = 0;
	};
};
