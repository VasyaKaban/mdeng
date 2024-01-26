#pragma once

#include <map>
#include "GBuffer.h"
#include "DefferedShader.h"

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
			RasterizationSubpass = 0,
			EvaluationSubpass,
			LastUnusedSubpassIndex
		};

		using FramebufferImageViewsArray = std::array<vk::ImageView,
													  AttachmentIndices::LastUnusedAttachmentIndex>;

		using ShadersMap = std::multimap<std::uint32_t,
										 std::unique_ptr<DefferedShader>,
										 std::greater<std::uint32_t>>;

		using SubpassShaderBindingsArray = std::array<ShadersMap, SubpassIndices::LastUnusedSubpassIndex>;

	private:

		DefferedPass(Device *_parent_device,
					 vk::RenderPass _renderpass,
					 vk::Framebuffer _framebuffer,
					 FramebufferImageViewsArray &&_framebuffer_image_views,
					 GBuffer &&_gbuffer) noexcept;

	public:

		~DefferedPass();
		DefferedPass(DefferedPass &&rpass) noexcept;
		DefferedPass & operator=(DefferedPass &&rpass) noexcept;

		static hrs::expected<DefferedPass, AllocationError>
		Create(Device *_device,
			   vk::Image light_pass_out_image,
			   vk::Format light_pass_out_image_format,
			   vk::Extent2D resolution);

		void Destroy();

		AllocationError Resize(vk::Extent2D resolution,
							   vk::Image light_pass_out_image,
							   vk::Format light_pass_out_image_format);

		vk::Result Render(vk::CommandBuffer command_buffer);

		bool IsCreated() const noexcept;

		const SubpassShaderBindingsArray & GetSubpassShaderBindings() const noexcept;

		void AddShader(SubpassIndices subpass_index,
					   DefferedShader *shader,
					   std::uint32_t priority = 0);

		void DropShaders(SubpassIndices subpass_index,
						 ShadersMap::const_iterator start,
						 ShadersMap::const_iterator end);

	private:

		static hrs::expected<GBuffer, AllocationError> create_gbuffer(Device *device,
																	  vk::Extent2D resolution);

		static vk::ResultValue<vk::UniqueRenderPass>
		create_renderpass(Device *device,
						  const GBuffer &_gbuffer,
						  vk::Format light_pass_out_image_format) noexcept;

		static hrs::expected<FramebufferImageViewsArray, vk::Result>
		create_framebuffer_image_views(Device *device,
									   const GBuffer &_gbuffer,
									   vk::Image light_pass_out_image,
									   vk::Format light_pass_out_image_format);

		static vk::ResultValue<vk::UniqueFramebuffer>
		create_framebuffer(Device *device,
						   vk::RenderPass rpass,
						   const FramebufferImageViewsArray &image_views,
						   vk::Extent2D resolution) noexcept;


		Device *parent_device;
		vk::RenderPass renderpass;
		vk::Framebuffer framebuffer;
		FramebufferImageViewsArray framebuffer_image_views;
		GBuffer gbuffer;
		SubpassShaderBindingsArray subpass_shader_bindings;
	};
};
