#pragma once

#include "../../Shader/Shader.h"

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
	};
};
