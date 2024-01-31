#pragma once

#include "../Camera/ComputedCamera.h"

namespace FireLand
{
	class Scene
	{
	public:
		virtual ~Scene() {}

		virtual void Render(vk::CommandBuffer command_buffer,
							vk::RenderPass renderpass,
							std::uint32_t frame_index,
							vk::DescriptorSet globals_set,
							vk::DescriptorSet renderpass_set,
							std::uint32_t subpass_index,
							const ComputedCamera &camera) const noexcept = 0;
	};
};
