#include "TestEvalShader.h"
#include "../../../../../hrs/debug.hpp"
#include "../../../../../Vulkan/UnexpectedVkResult.hpp"

namespace FireLand
{
	void TestEvalShader::init(vk::Pipeline _pipeline, vk::PipelineLayout _pipeline_layout) noexcept
	{
		pipeline = _pipeline;
		pipeline_layout = _pipeline_layout;
	}

	TestEvalShader::TestEvalShader(vk::Device _parent_device) noexcept
		:parent_device(_parent_device)
	{
		hrs::assert_true_debug(_parent_device, "Parent device isn't created yet!");
	}

	TestEvalShader::~TestEvalShader()
	{
		TestEvalShader::Destroy();
	}

	TestEvalShader::TestEvalShader(TestEvalShader &&eval_sh) noexcept
		: parent_device(eval_sh.parent_device),
		  pipeline(eval_sh.pipeline),
		  pipeline_layout(eval_sh.pipeline_layout)
	{
		eval_sh.pipeline = VK_NULL_HANDLE;
		eval_sh.pipeline_layout = VK_NULL_HANDLE;
	}

	TestEvalShader & TestEvalShader::operator=(TestEvalShader &&eval_sh) noexcept
	{
		Destroy();

		parent_device = eval_sh.parent_device;
		pipeline = eval_sh.pipeline;
		pipeline_layout = eval_sh.pipeline_layout;

		eval_sh.pipeline = VK_NULL_HANDLE;
		eval_sh.pipeline_layout = VK_NULL_HANDLE;

		return *this;
	}

	void TestEvalShader::Destroy()
	{
		if(!TestEvalShader::IsCreated())
			return;

		parent_device.destroy(pipeline);
		parent_device.destroy(pipeline_layout);

		pipeline_layout = VK_NULL_HANDLE;
		pipeline = VK_NULL_HANDLE;
	}

	bool TestEvalShader::IsCreated() const noexcept
	{
		return pipeline;
	}

	vk::Pipeline TestEvalShader::GetPipeline() const noexcept
	{
		return pipeline;
	}

	vk::PipelineLayout TestEvalShader::GetPipelineLayout() const noexcept
	{
		return pipeline_layout;
	}

	bool TestEvalShader::IsSceneIndependent() const noexcept
	{
		return true;
	}

	hrs::unexpected_result
	TestEvalShader::Recreate(const std::map<vk::ShaderStageFlagBits, ShaderInfo> shader_infos,
								 vk::RenderPass renderpass,
								 std::uint32_t subpass_index,
								 vk::DescriptorSetLayout globals_set_layout,
								 vk::DescriptorSetLayout renderpass_set_layout,
								 const vk::Extent2D &resolution)
	{
		Destroy();

		const std::array descriptor_set_layouts =
		{
			//globals_set_layout,
			renderpass_set_layout
		};

		const vk::PipelineLayoutCreateInfo layout_info({},
													   descriptor_set_layouts,
													   {});

		auto[u_layout_res, u_layout] = parent_device.createPipelineLayoutUnique(layout_info);
		if(u_layout_res != vk::Result::eSuccess)
			return UnexpectedVkResult(u_layout_res);

		auto vertex_stage_it = shader_infos.find(vk::ShaderStageFlagBits::eVertex);
		auto fragment_stage_it = shader_infos.find(vk::ShaderStageFlagBits::eFragment);
		if(vertex_stage_it == shader_infos.end() || fragment_stage_it == shader_infos.end())
			return UnexpectedShaderResult(ShaderResult::NoRequiredShaderStage);

		const std::array stages =
		{
			vk::PipelineShaderStageCreateInfo({},
											  vk::ShaderStageFlagBits::eVertex,
											  vertex_stage_it->second.shader_module,
											  vertex_stage_it->second.entry_name.c_str(),
											  {}),
			vk::PipelineShaderStageCreateInfo({},
											  vk::ShaderStageFlagBits::eFragment,
											  fragment_stage_it->second.shader_module,
											  fragment_stage_it->second.entry_name.c_str(),
											  {}),
		};

		const vk::PipelineVertexInputStateCreateInfo vertex_input_state_info({}, {}, {});
		const vk::PipelineInputAssemblyStateCreateInfo
			input_assembly_state_info({},
									  vk::PrimitiveTopology::eTriangleList,
									  VK_FALSE);

		const vk::PipelineViewportStateCreateInfo viewport_state_info({},
																	  1,
																	  {},
																	  1,
																	  {});

		const vk::PipelineRasterizationStateCreateInfo
			rasterization_state_info({},
									 VK_FALSE,
									 VK_FALSE,
									 vk::PolygonMode::eFill,
									 vk::CullModeFlagBits::eBack,
									 vk::FrontFace::eClockwise,
									 VK_FALSE,
									 {},
									 {},
									 {},
									 1.0f);

		const vk::PipelineMultisampleStateCreateInfo
			multisample_state_info({},
								   vk::SampleCountFlagBits::e1,
								   VK_FALSE,
								   {}, {}, {}, {});

		const vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_info({},
																			   VK_FALSE,
																			   VK_FALSE,
																			   {},
																			   VK_FALSE,
																			   VK_FALSE/*stencil test*/);

		const std::array attachments =
		{
			vk::PipelineColorBlendAttachmentState(VK_FALSE,
												  vk::BlendFactor::eZero,
												  vk::BlendFactor::eZero,
												  vk::BlendOp::eAdd,
												  vk::BlendFactor::eZero,
												  vk::BlendFactor::eZero,
												  vk::BlendOp::eAdd,
												  vk::ColorComponentFlagBits::eA |
												  vk::ColorComponentFlagBits::eR |
												  vk::ColorComponentFlagBits::eG |
												  vk::ColorComponentFlagBits::eB)
		};

		const vk::PipelineColorBlendStateCreateInfo
			color_blend_state_info({},
								   VK_FALSE,
								   vk::LogicOp::eClear,
								   attachments);

		const std::array dynamic_states =
		{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		const vk::PipelineDynamicStateCreateInfo dynamic_state_info({},
																	dynamic_states);


		const vk::GraphicsPipelineCreateInfo pipeline_info({},
														   stages,
														   &vertex_input_state_info,
														   &input_assembly_state_info,
														   {}/*tessellation*/,
														   &viewport_state_info,
														   &rasterization_state_info,
														   &multisample_state_info,
														   &depth_stencil_state_info,
														   &color_blend_state_info,
														   &dynamic_state_info,
														   u_layout.get(),
														   renderpass,
														   subpass_index,
														   {},
														   {});

		auto [_pipeline_res, _pipeline] = parent_device.createGraphicsPipeline({}, pipeline_info);
		if(_pipeline_res != vk::Result::eSuccess)
			return UnexpectedVkResult(_pipeline_res);

		init(_pipeline, u_layout.release());

		return {};
	}

	void TestEvalShader::Bind(vk::CommandBuffer command_buffer,
							  vk::DescriptorSet globals_set,
							  vk::DescriptorSet renderpass_set) const noexcept
	{
		const std::array sets =
		{
			//globals_set,
			renderpass_set
		};

		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
									pipeline);

		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
										  pipeline_layout,
										  0,
										  sets,
										  {});
	}

	void TestEvalShader::PushConstants(vk::CommandBuffer command_buffer,
									   const std::byte *data) const noexcept
	{

	}
};

