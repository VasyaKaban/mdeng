#pragma once

#include <cstdint>
#include "../../../Vulkan/VulkanInclude.hpp"

namespace FireLand
{
	class RenderInputs;

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
							const RenderInputs &inputs) const noexcept = 0;
	};
};
