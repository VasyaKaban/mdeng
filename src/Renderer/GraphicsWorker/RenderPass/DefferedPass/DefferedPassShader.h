#pragma once

#include "../../../Shader/Shader.h"

namespace FireLand
{
	class DefferedPassShader : public Shader
	{
	public:
		virtual hrs::unexpected_result
		Recreate(const std::map<vk::ShaderStageFlagBits, ShaderInfo> shader_infos,
				 vk::RenderPass renderpass,
				 std::uint32_t subpass_index,
				 vk::DescriptorSetLayout globals_set_layout,
				 vk::DescriptorSetLayout renderpass_set_layout,
				 const vk::Extent2D &resolution) = 0;

		virtual void Bind(vk::CommandBuffer command_buffer,
						  vk::DescriptorSet globals_set,
						  vk::DescriptorSet renderpass_set) const noexcept = 0;

		virtual void PushConstants(vk::CommandBuffer command_buffer,
								   const std::byte *data) const noexcept = 0;
	};
};
