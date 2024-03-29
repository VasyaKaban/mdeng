#pragma once

#include "../DefferedPassShader.h"
#include "../../../../../hrs/non_creatable.hpp"

namespace FireLand
{
	class TestEvalShader : public DefferedPassShader, public hrs::non_copyable
	{
		void init(vk::Pipeline _pipeline, vk::PipelineLayout _pipeline_layout) noexcept;

	public:

		TestEvalShader(vk::Device _parent_device) noexcept;
		virtual ~TestEvalShader() override;
		TestEvalShader(TestEvalShader &&eval_sh) noexcept;
		TestEvalShader & operator=(TestEvalShader &&eval_sh) noexcept;

		virtual void Destroy() override;

		virtual bool IsCreated() const noexcept override;

		virtual vk::Pipeline GetPipeline() const noexcept override;
		virtual vk::PipelineLayout GetPipelineLayout() const noexcept override;

		virtual bool IsSceneIndependent() const noexcept override;

		virtual hrs::unexpected_result
		Recreate(const std::map<vk::ShaderStageFlagBits, ShaderInfo> shader_infos,
				 vk::RenderPass renderpass,
				 std::uint32_t subpass_index,
				 vk::DescriptorSetLayout globals_set_layout,
				 vk::DescriptorSetLayout renderpass_set_layout,
				 const vk::Extent2D &resolution) override;

		virtual void Bind(vk::CommandBuffer command_buffer,
						  vk::DescriptorSet globals_set,
						  vk::DescriptorSet renderpass_set) const noexcept override;

		virtual void PushConstants(vk::CommandBuffer command_buffer,
								   const std::byte *data) const noexcept override;

	private:

		vk::Device parent_device;
		vk::Pipeline pipeline;
		vk::PipelineLayout pipeline_layout;
	};
};

