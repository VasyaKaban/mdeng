#pragma once

#include <map>
#include "GBuffer.h"
#include "../../../../hrs/unexpected_result.hpp"
#include "../../../../Scene/Scene.h"
#include "../../../../Camera/ComputedCamera.h"
#include "DefferedPassShader.h"
#include "../../../Shader/ShaderInfoNode.hpp"

namespace FireLand
{
	class DefferedPass : public hrs::non_copyable
	{
	public:
		enum AttachmentIndices
		{
			GBufferColor = 0,
			GBufferDepthStencil,
			EvaluationImage,
			LastUnusedAttachmentIndex
		};

		enum SubpassIndices
		{
			RasterizationSubpassIndex = 0,
			EvaluationSubpassIndex,
			LastUnusedSubpassIndex
		};

		using FramebufferImageViewsArray = std::array<vk::ImageView,
													  AttachmentIndices::LastUnusedAttachmentIndex>;

		using ShaderSubpassBindingsArray =
			std::array<std::vector<ShaderInfoNode<DefferedPassShader>>,
					   SubpassIndices::LastUnusedSubpassIndex>;

		using AttachmentFormatsArray = std::array<vk::Format, AttachmentIndices::LastUnusedAttachmentIndex>;

	private:

		void init(GBuffer &&_gbuffer,
				  vk::RenderPass _renderpass,
				  vk::Framebuffer _framebuffer,
				  FramebufferImageViewsArray &&_framebuffer_image_views,
				  const AttachmentFormatsArray &_attachment_formats,
				  vk::DescriptorPool _descriptor_pool,
				  vk::DescriptorSetLayout _descriptor_set_layout,
				  std::vector<vk::DescriptorSet> &&_descriptor_sets) noexcept;

	public:

		DefferedPass(Device *_parent_device) noexcept;

		~DefferedPass();
		DefferedPass(DefferedPass &&rpass) noexcept;
		DefferedPass & operator=(DefferedPass &&rpass) noexcept;

		void Destroy();
		void DestroyWithoutShaders();

		hrs::unexpected_result Resize(const vk::Extent2D &resolution,
									  vk::Image evaluation_image,
									  vk::Format evaluation_image_format,
									  vk::DescriptorSetLayout globals_layout,
									  std::uint32_t frame_count);

		void Render(vk::CommandBuffer command_buffer,
					std::uint32_t frame_index,
					vk::DescriptorSet globals_set,
					const ComputedCamera &camera,
					const Scene *scene);

		bool IsCreated() const noexcept;

		const AttachmentFormatsArray & GetAttachmentFormats() const noexcept;

		hrs::unexpected_result AddShader(SubpassIndices subpass_index,
										 DefferedPassShader *shader,
										 const std::map<vk::ShaderStageFlagBits, ShaderInfo> shader_infos);

		const GBuffer &GetGBuffer() const noexcept
		{
			return gbuffer;
		}

	private:

		hrs::expected<GBuffer, hrs::unexpected_result> create_gbuffer(const vk::Extent2D &resolution);

		vk::ResultValue<vk::UniqueRenderPass>
		create_renderpass(const GBuffer &_gbuffer, vk::Format evaluation_image_format) noexcept;

		hrs::expected<FramebufferImageViewsArray, vk::Result>
		create_framebuffer_image_views(const GBuffer &_gbuffer,
									   vk::Image evaluation_image,
									   vk::Format evaluation_image_format);

		vk::ResultValue<vk::UniqueFramebuffer>
		create_framebuffer(vk::RenderPass rpass,
						   const FramebufferImageViewsArray &image_views,
						   const vk::Extent2D &resolution) noexcept;

		AttachmentFormatsArray create_attachment_formats(const GBuffer &_gbuffer,
														 vk::Format evaluation_image_format);

		using create_descriptor_sets_result_t = hrs::expected<std::tuple<vk::UniqueDescriptorPool,
																		 vk::UniqueDescriptorSetLayout,
																		 std::vector<vk::DescriptorSet>>,
															  vk::Result>;


		create_descriptor_sets_result_t create_descriptor_sets(const FramebufferImageViewsArray &views,
															   std::uint32_t frame_count);

		Device *parent_device;
		vk::RenderPass renderpass;
		vk::Framebuffer framebuffer;
		FramebufferImageViewsArray framebuffer_image_views;
		GBuffer gbuffer;
		AttachmentFormatsArray attachment_formats;

		vk::DescriptorPool descriptor_pool;
		vk::DescriptorSetLayout descriptor_set_layout;
		std::vector<vk::DescriptorSet> descriptor_sets;

		ShaderSubpassBindingsArray shader_bindings;
	};
};
