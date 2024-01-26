#include "DefferedPass.h"
#include "../../../../hrs/scoped_call.hpp"

namespace FireLand
{
	DefferedPass::DefferedPass(Device *_parent_device,
							   vk::RenderPass _renderpass,
							   vk::Framebuffer _framebuffer,
							   FramebufferImageViewsArray &&_framebuffer_image_views,
							   GBuffer &&_gbuffer) noexcept
		: parent_device(_parent_device),
		  renderpass(_renderpass),
		  framebuffer(_framebuffer),
		  framebuffer_image_views(std::move(_framebuffer_image_views)),
		  gbuffer(std::move(_gbuffer)) {}

	DefferedPass::~DefferedPass()
	{
		Destroy();
	}

	DefferedPass::DefferedPass(DefferedPass &&rpass) noexcept
		: parent_device(rpass.parent_device),
		  renderpass(rpass.renderpass),
		  framebuffer(rpass.framebuffer),
		  framebuffer_image_views(std::move(rpass.framebuffer_image_views)),
		  gbuffer(std::move(rpass.gbuffer)),
		  subpass_shader_bindings(std::move(rpass.subpass_shader_bindings))
	{
		rpass.renderpass = VK_NULL_HANDLE;
		rpass.framebuffer = VK_NULL_HANDLE;
		std::ranges::fill(rpass.framebuffer_image_views, VK_NULL_HANDLE);
	}

	DefferedPass & DefferedPass::operator=(DefferedPass &&rpass) noexcept
	{
		Destroy();

		parent_device = rpass.parent_device;
		renderpass = rpass.renderpass;
		framebuffer = rpass.framebuffer;
		framebuffer_image_views = std::move(rpass.framebuffer_image_views);
		gbuffer = std::move(rpass.gbuffer);
		subpass_shader_bindings = std::move(rpass.subpass_shader_bindings);

		rpass.renderpass = VK_NULL_HANDLE;
		rpass.framebuffer = VK_NULL_HANDLE;
		std::ranges::fill(rpass.framebuffer_image_views, VK_NULL_HANDLE);

		return *this;
	}

	hrs::expected<DefferedPass, AllocationError>
	DefferedPass::Create(Device *_device,
						 vk::Image light_pass_out_image,
						 vk::Format light_pass_out_image_format,
						 vk::Extent2D resolution)
	{
		hrs::assert_true_debug(_device, "Parent device is points to null!");
		hrs::assert_true_debug(_device->GetDevice(), "Parent device isn't created yet!");

		if(resolution.width == resolution.height && resolution.width == 0)
		{
			return DefferedPass(_device,
								{},
								{},
								{},
								GBuffer::CreateNull(_device));
		}

		auto gbuffer_exp = create_gbuffer(_device, resolution);
		if(!gbuffer_exp)
			return gbuffer_exp.error();

		auto [u_renderpass_res, u_renderpass] = create_renderpass(_device,
																  gbuffer_exp.value(),
																  light_pass_out_image_format);

		if(u_renderpass_res != vk::Result::eSuccess)
			return {u_renderpass_res};

		auto image_views_exp = create_framebuffer_image_views(_device,
															  gbuffer_exp.value(),
															  light_pass_out_image,
															  light_pass_out_image_format);

		hrs::scoped_call image_views_dtor([&image_views_exp, _device]()
		{
			for(auto image_view : image_views_exp.value())
				_device->GetDevice().destroy(image_view);
		});

		if(!image_views_exp)
			return {image_views_exp.error()};

		auto [u_framebuffer_res, u_framebuffer] = create_framebuffer(_device,
																	 u_renderpass.get(),
																	 image_views_exp.value(),
																	 resolution);

		if(u_framebuffer_res != vk::Result::eSuccess)
			return {u_framebuffer_res};

		image_views_dtor.Drop();

		return DefferedPass(_device,
							u_renderpass.release(),
							u_framebuffer.release(),
							std::move(image_views_exp.value()),
							std::move(gbuffer_exp.value()));
	}

	void DefferedPass::Destroy()
	{
		if(!IsCreated())
			return;

		vk::Device device_handle = parent_device->GetDevice();
		for(auto &subpass_binding : subpass_shader_bindings)
			subpass_binding.clear();

		for(auto &image_view : framebuffer_image_views)
		{
			device_handle.destroy(image_view);
			image_view = VK_NULL_HANDLE;
		}

		device_handle.destroy(framebuffer);
		device_handle.destroy(renderpass);
		gbuffer.Destroy();

		framebuffer = VK_NULL_HANDLE;
		renderpass = VK_NULL_HANDLE;
	}

	AllocationError DefferedPass::Resize(vk::Extent2D resolution,
										 vk::Image light_pass_out_image,
										 vk::Format light_pass_out_image_format)
	{
		hrs::assert_true_debug(parent_device != nullptr,
							   "No connected parent device!");

		if(gbuffer.GetResolution() == resolution)
			return vk::Result::eSuccess;

		if(resolution.width == resolution.height && resolution.width == 0)
		{
			vk::Device device_handle = parent_device->GetDevice();
			for(auto &image_view : framebuffer_image_views)
			{
				device_handle.destroy(image_view);
				image_view = VK_NULL_HANDLE;
			}

			device_handle.destroy(framebuffer);
			device_handle.destroy(renderpass);
			gbuffer.Destroy();

			framebuffer = VK_NULL_HANDLE;
			renderpass = VK_NULL_HANDLE;
		}
		else
		{
			auto gbuffer_exp = create_gbuffer(parent_device, resolution);
			if(!gbuffer_exp)
				return gbuffer_exp.error();

			auto [u_renderpass_res, u_renderpass] = create_renderpass(parent_device,
																	  gbuffer_exp.value(),
																	  light_pass_out_image_format);

			if(u_renderpass_res != vk::Result::eSuccess)
				return {u_renderpass_res};

			auto image_views_exp = create_framebuffer_image_views(parent_device,
																  gbuffer_exp.value(),
																  light_pass_out_image,
																  light_pass_out_image_format);

			hrs::scoped_call image_views_dtor([&image_views_exp, this]()
			{
				for(auto image_view : image_views_exp.value())
					parent_device->GetDevice().destroy(image_view);
			});

			if(!image_views_exp)
				return {image_views_exp.error()};

			auto [u_framebuffer_res, u_framebuffer] = create_framebuffer(parent_device,
																		 u_renderpass.get(),
																		 image_views_exp.value(),
																		 resolution);

			if(u_framebuffer_res != vk::Result::eSuccess)
				return {u_framebuffer_res};

			image_views_dtor.Drop();

			renderpass = u_renderpass.release();
			framebuffer = u_framebuffer.release();
			framebuffer_image_views = std::move(image_views_exp.value());
			gbuffer = std::move(gbuffer_exp.value());
		}

		for(auto &subpass_binding : subpass_shader_bindings)
			for(auto &shader : subpass_binding)
				shader.second->NotifyResize(resolution);

		return vk::Result::eSuccess;
	}

	vk::Result DefferedPass::Render(vk::CommandBuffer command_buffer)
	{
		hrs::assert_true_debug(IsCreated(), "DefferedPass isn't create yet!");
		std::array<vk::ClearValue, AttachmentIndices::LastUnusedAttachmentIndex> clear_values;
		clear_values[AttachmentIndices::GBufferColor] = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f);
		clear_values[AttachmentIndices::GBufferDepthStencil] = vk::ClearDepthStencilValue(1.0f, 0);
		clear_values[AttachmentIndices::EvaluationImage] = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f);

		vk::RenderPassBeginInfo begin_info(renderpass,
										   framebuffer,
										   vk::Rect2D(0, gbuffer.GetResolution()),
										   clear_values);
		command_buffer.beginRenderPass(begin_info, vk::SubpassContents::eInline);

		for(std::size_t i = 0; i < SubpassIndices::LastUnusedSubpassIndex; i++)
		{
			if(i != 0)
				command_buffer.nextSubpass(vk::SubpassContents::eInline);

			for(auto &shader : subpass_shader_bindings[i])
				shader.second->Render(command_buffer);
		}

		command_buffer.endRenderPass();

		return vk::Result::eSuccess;
	}

	bool DefferedPass::IsCreated() const noexcept
	{
		return renderpass;
	}

	const DefferedPass::SubpassShaderBindingsArray & DefferedPass::GetSubpassShaderBindings() const noexcept
	{
		return subpass_shader_bindings;
	}

	void DefferedPass::AddShader(SubpassIndices subpass_index,
								 DefferedShader *shader,
								 std::uint32_t priority)
	{
		subpass_shader_bindings[subpass_index].insert(std::pair{priority, shader});
	}

	void DefferedPass::DropShaders(SubpassIndices subpass_index,
								   ShadersMap::const_iterator start,
								   ShadersMap::const_iterator end)
	{
		subpass_shader_bindings[subpass_index].erase(start, end);
	}

	hrs::expected<GBuffer, AllocationError> DefferedPass::create_gbuffer(Device *device, vk::Extent2D resolution)
	{
		constexpr static std::array color_buffer_formats =
		{
			vk::Format::eR8G8B8A8Unorm,
			vk::Format::eR8G8B8A8Snorm,
			vk::Format::eR8G8B8A8Uscaled,
			vk::Format::eR8G8B8A8Sscaled,
			vk::Format::eR8G8B8A8Uint,
			vk::Format::eR8G8B8A8Sint,
			vk::Format::eR8G8B8A8Srgb,
			vk::Format::eB8G8R8A8Unorm,
			vk::Format::eB8G8R8A8Snorm,
			vk::Format::eB8G8R8A8Uscaled,
			vk::Format::eB8G8R8A8Sscaled,
			vk::Format::eB8G8R8A8Uint,
			vk::Format::eB8G8R8A8Sint,
			vk::Format::eB8G8R8A8Srgb,
			vk::Format::eA8B8G8R8UnormPack32,
			vk::Format::eA8B8G8R8SnormPack32,
			vk::Format::eA8B8G8R8UscaledPack32,
			vk::Format::eA8B8G8R8SscaledPack32,
			vk::Format::eA8B8G8R8UintPack32,
			vk::Format::eA8B8G8R8SintPack32,
			vk::Format::eA8B8G8R8SrgbPack32,
			vk::Format::eA2R10G10B10UnormPack32,
			vk::Format::eA2R10G10B10SnormPack32,
			vk::Format::eA2R10G10B10UscaledPack32,
			vk::Format::eA2R10G10B10SscaledPack32,
			vk::Format::eA2R10G10B10UintPack32,
			vk::Format::eA2R10G10B10SintPack32,
			vk::Format::eA2B10G10R10UnormPack32,
			vk::Format::eA2B10G10R10SnormPack32,
			vk::Format::eA2B10G10R10UscaledPack32,
			vk::Format::eA2B10G10R10SscaledPack32,
			vk::Format::eA2B10G10R10UintPack32,
			vk::Format::eA2B10G10R10SintPack32
		};

		constexpr static std::array depth_stencil_buffer_formats =
		{
			vk::Format::eD24UnormS8Uint,
			vk::Format::eD32Sfloat,
			vk::Format::eX8D24UnormPack32,
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD16UnormS8Uint,
			vk::Format::eD16Unorm
		};

		vk::PhysicalDevice ph_device = device->GetPhysicalDevice();

		auto find_format = [ph_device]<std::size_t N>(const std::array<vk::Format, N> &formats,
													   vk::FormatFeatureFlags features)
		{
			for(const auto format : formats)
			{
				auto props = ph_device.getFormatProperties(format);
				if(props.optimalTilingFeatures & features)
					return format;
			}

			return vk::Format::eUndefined;
		};

		std::array<vk::FormatFeatureFlags, GBuffer::BufferIndices::LastUnusedBufferIndex> formats_fetures;
		formats_fetures[GBuffer::BufferIndices::ColorBuffer] =
			vk::FormatFeatureFlagBits::eColorAttachment;
		formats_fetures[GBuffer::BufferIndices::DepthStencilBuffer] =
			vk::FormatFeatureFlagBits::eDepthStencilAttachment;

		std::array<vk::Format, GBuffer::BufferIndices::LastUnusedBufferIndex> choosed_formats;

		choosed_formats[GBuffer::BufferIndices::ColorBuffer] =
			find_format(color_buffer_formats,
						formats_fetures[GBuffer::BufferIndices::ColorBuffer]);

		choosed_formats[GBuffer::BufferIndices::DepthStencilBuffer] =
			find_format(color_buffer_formats,
						formats_fetures[GBuffer::BufferIndices::DepthStencilBuffer]);

		for(const auto format : choosed_formats)
			if(format == vk::Format::eUndefined)
				return {vk::Result::eErrorFormatNotSupported};

		GBufferImageParams color_buffer_params(vk::ImageUsageFlagBits::eColorAttachment |
											   vk::ImageUsageFlagBits::eInputAttachment,
											   choosed_formats[GBuffer::BufferIndices::ColorBuffer],
											   vk::ImageLayout::eColorAttachmentOptimal);

		GBufferImageParams depth_stencil_buffer_params(vk::ImageUsageFlagBits::eDepthStencilAttachment |
													   vk::ImageUsageFlagBits::eInputAttachment,
													   choosed_formats[GBuffer::BufferIndices::DepthStencilBuffer],
													   vk::ImageLayout::eDepthStencilAttachmentOptimal);

		return GBuffer::Create(device,
							   color_buffer_params,
							   depth_stencil_buffer_params,
							   resolution);
	}

	vk::ResultValue<vk::UniqueRenderPass>
	DefferedPass::create_renderpass(Device *device,
									const GBuffer &_gbuffer,
									vk::Format light_pass_out_image_format) noexcept
	{
		std::array<vk::AttachmentDescription, AttachmentIndices::LastUnusedAttachmentIndex> attachments;
		attachments[AttachmentIndices::GBufferColor] =
			vk::AttachmentDescription({},
									  _gbuffer.GetBuffer(GBuffer::ColorBuffer).format,
									  vk::SampleCountFlagBits::e1,
									  vk::AttachmentLoadOp::eClear,
									  vk::AttachmentStoreOp::eStore,
									  vk::AttachmentLoadOp::eDontCare,
									  vk::AttachmentStoreOp::eDontCare,
									  vk::ImageLayout::eUndefined,
									  vk::ImageLayout::eColorAttachmentOptimal);

		attachments[AttachmentIndices::GBufferDepthStencil] =
			vk::AttachmentDescription({},
									  _gbuffer.GetBuffer(GBuffer::DepthStencilBuffer).format,
									  vk::SampleCountFlagBits::e1,
									  vk::AttachmentLoadOp::eClear,
									  vk::AttachmentStoreOp::eStore,
									  vk::AttachmentLoadOp::eDontCare,
									  vk::AttachmentStoreOp::eDontCare,
									  vk::ImageLayout::eUndefined,
									  vk::ImageLayout::eDepthStencilAttachmentOptimal);

		attachments[AttachmentIndices::EvaluationImage] =
			vk::AttachmentDescription({},
									  light_pass_out_image_format,
									  vk::SampleCountFlagBits::e1,
									  vk::AttachmentLoadOp::eClear,
									  vk::AttachmentStoreOp::eStore,
									  vk::AttachmentLoadOp::eDontCare,
									  vk::AttachmentStoreOp::eDontCare,
									  vk::ImageLayout::eUndefined,
									  vk::ImageLayout::eColorAttachmentOptimal);

		const std::array rasterization_subpass_color_attachments =
		{
			vk::AttachmentReference(AttachmentIndices::GBufferColor,
									vk::ImageLayout::eColorAttachmentOptimal)
		};

		const std::array rasterization_subpass_depth_stencil_attachments =
		{
			vk::AttachmentReference(AttachmentIndices::GBufferDepthStencil,
									vk::ImageLayout::eDepthStencilAttachmentOptimal)
		};

		const std::array evaluation_subpass_input_attachments =
		{
			vk::AttachmentReference(AttachmentIndices::GBufferColor,
									vk::ImageLayout::eColorAttachmentOptimal),
			vk::AttachmentReference(AttachmentIndices::GBufferDepthStencil,
									vk::ImageLayout::eDepthStencilAttachmentOptimal)
		};

		const std::array evaluation_subpass_color_attachments =
		{
			vk::AttachmentReference(AttachmentIndices::EvaluationImage,
									vk::ImageLayout::eColorAttachmentOptimal),
		};

		std::array<vk::SubpassDescription, SubpassIndices::LastUnusedSubpassIndex> subpasses;
		subpasses[SubpassIndices::RasterizationSubpass] =
			vk::SubpassDescription({},
								   vk::PipelineBindPoint::eGraphics,
								   {},
								   rasterization_subpass_color_attachments,
								   {},
								   rasterization_subpass_depth_stencil_attachments.data(),
								   {});
		subpasses[SubpassIndices::EvaluationSubpass] =
			vk::SubpassDescription({},
								   vk::PipelineBindPoint::eGraphics,
								   evaluation_subpass_input_attachments,
								   evaluation_subpass_color_attachments,
								   {},
								   {},
								   {});

		const std::array subpass_dependencies =
		{
			vk::SubpassDependency(VK_SUBPASS_EXTERNAL,
								  SubpassIndices::RasterizationSubpass,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::PipelineStageFlagBits::eEarlyFragmentTests,
								  vk::AccessFlagBits::eColorAttachmentWrite |
									  vk::AccessFlagBits::eDepthStencilAttachmentWrite,
								  vk::AccessFlagBits::eColorAttachmentRead |
									  vk::AccessFlagBits::eDepthStencilAttachmentRead,
								  vk::DependencyFlagBits::eByRegion),

			vk::SubpassDependency(SubpassIndices::RasterizationSubpass,
								  SubpassIndices::EvaluationSubpass,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::AccessFlagBits::eColorAttachmentWrite,
								  vk::AccessFlagBits::eColorAttachmentRead,
								  vk::DependencyFlagBits::eByRegion),

			vk::SubpassDependency(SubpassIndices::EvaluationSubpass,
								  VK_SUBPASS_EXTERNAL,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::PipelineStageFlagBits::eTopOfPipe,
								  vk::AccessFlagBits::eColorAttachmentWrite,
								  vk::AccessFlagBits::eColorAttachmentWrite |
									  vk::AccessFlagBits::eColorAttachmentRead,
								  vk::DependencyFlagBits::eByRegion)
		};

		vk::RenderPassCreateInfo info({},
									  attachments,
									  subpasses,
									  subpass_dependencies);

		return device->GetDevice().createRenderPassUnique(info);
	}

	hrs::expected<DefferedPass::FramebufferImageViewsArray, vk::Result>
	DefferedPass::create_framebuffer_image_views(Device *device,
								   const GBuffer &_gbuffer,
								   vk::Image light_pass_out_image,
								   vk::Format light_pass_out_image_format)

	{
		vk::ImageViewCreateInfo info({},
									 _gbuffer.GetBuffer(GBuffer::BufferIndices::ColorBuffer).image.image,
									 vk::ImageViewType::e2D,
									 _gbuffer.GetBuffer(GBuffer::BufferIndices::ColorBuffer).format,
									 vk::ComponentMapping(/*all identity*/),
									 vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor,
															   0,
															   1,
															   0,
															   1));

		FramebufferImageViewsArray _views;
		hrs::scoped_call views_dtor([&_views, device]()
		{
			for(auto &view : _views)
				device->GetDevice().destroy(view);
		});

		auto [color_view_res, color_view] = device->GetDevice().createImageView(info);
		if(color_view_res != vk::Result::eSuccess)
			return color_view_res;

		_views[AttachmentIndices::GBufferColor] = color_view;

		info.image = _gbuffer.GetBuffer(GBuffer::BufferIndices::DepthStencilBuffer).image.image;
		info.format = _gbuffer.GetBuffer(GBuffer::BufferIndices::DepthStencilBuffer).format;
		info.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth |
															  vk::ImageAspectFlagBits::eStencil,
														  0,
														  1,
														  0,
														  1);

		auto [depth_view_res, depth_view] = device->GetDevice().createImageView(info);
		if(depth_view_res != vk::Result::eSuccess)
			return depth_view_res;

		_views[AttachmentIndices::GBufferDepthStencil] = depth_view;

		info.image = light_pass_out_image;
		info.format = light_pass_out_image_format;
		info.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor,
														  0,
														  1,
														  0,
														  1);

		auto [eval_view_res, eval_view] = device->GetDevice().createImageView(info);
		if(eval_view_res != vk::Result::eSuccess)
			return eval_view_res;

		_views[AttachmentIndices::EvaluationImage] = eval_view;

		views_dtor.Drop();
		return _views;
	}

	vk::ResultValue<vk::UniqueFramebuffer>
	DefferedPass::create_framebuffer(Device *device,
									 vk::RenderPass rpass,
									 const FramebufferImageViewsArray &image_views,
									 vk::Extent2D resolution) noexcept
	{
		vk::FramebufferCreateInfo info({},
									   rpass,
									   image_views,
									   resolution.width,
									   resolution.height,
									   1);

		return device->GetDevice().createFramebufferUnique(info);
	}
};
