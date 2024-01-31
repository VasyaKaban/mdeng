#include "DefferedPass.h"
#include "../../../../hrs/scoped_call.hpp"
#include "../../../../Vulkan/VulkanUtils.hpp"
#include "../../../../Vulkan/UnexpectedVkResult.hpp"
#include "../../../../Vulkan/VulkanFormatUtils.hpp"

namespace FireLand
{
	void DefferedPass::init(GBuffer &&_gbuffer,
							vk::RenderPass _renderpass,
							vk::Framebuffer _framebuffer,
							FramebufferImageViewsArray &&_framebuffer_image_views,
							const AttachmentFormatsArray &_attachment_formats,
							vk::DescriptorPool _descriptor_pool,
							vk::DescriptorSetLayout _descriptor_set_layout,
							std::vector<vk::DescriptorSet> &&_descriptor_sets) noexcept
	{
		renderpass = _renderpass;
		framebuffer = _framebuffer;
		framebuffer_image_views = std::move(_framebuffer_image_views);
		gbuffer = std::move(_gbuffer);
		attachment_formats = _attachment_formats;
		descriptor_pool = _descriptor_pool;
		descriptor_set_layout = _descriptor_set_layout;
		descriptor_sets = std::move(_descriptor_sets);
	}

	DefferedPass::DefferedPass(Device *_parent_device) noexcept
		: parent_device(_parent_device),
		  gbuffer(_parent_device) {}

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
		  attachment_formats(rpass.attachment_formats),
		  descriptor_pool(rpass.descriptor_pool),
		  descriptor_set_layout(rpass.descriptor_set_layout),
		  descriptor_sets(rpass.descriptor_sets)
	{
		rpass.renderpass = VK_NULL_HANDLE;
		rpass.framebuffer = VK_NULL_HANDLE;
		rpass.descriptor_pool = VK_NULL_HANDLE;
		rpass.descriptor_set_layout = VK_NULL_HANDLE;
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
		attachment_formats = rpass.attachment_formats;
		descriptor_pool = rpass.descriptor_pool;
		descriptor_set_layout = rpass.descriptor_set_layout;
		descriptor_sets = rpass.descriptor_sets;

		rpass.renderpass = VK_NULL_HANDLE;
		rpass.framebuffer = VK_NULL_HANDLE;
		rpass.descriptor_pool = VK_NULL_HANDLE;
		rpass.descriptor_set_layout = VK_NULL_HANDLE;
		std::ranges::fill(rpass.framebuffer_image_views, VK_NULL_HANDLE);

		return *this;
	}

	void DefferedPass::Destroy()
	{
		for(auto &shader_binding : shader_bindings)
			shader_binding.clear();

		DestroyWithoutShaders();
	}

	void DefferedPass::DestroyWithoutShaders()
	{
		if(!IsCreated())
			return;

		vk::Device device_handle = parent_device->GetDevice();

		device_handle.destroy(descriptor_set_layout);
		device_handle.resetDescriptorPool(descriptor_pool, {});
		device_handle.destroy(descriptor_pool);

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
		descriptor_pool = VK_NULL_HANDLE;
		descriptor_set_layout = VK_NULL_HANDLE;
		descriptor_sets.clear();
	}

	hrs::unexpected_result DefferedPass::Resize(const vk::Extent2D &resolution,
												vk::Image evaluation_image,
												vk::Format evaluation_image_format,
												vk::DescriptorSetLayout globals_layout,
												std::uint32_t frame_count)
	{
		DestroyWithoutShaders();

		if(!IsBadExtent(resolution))
		{
			auto gbuffer_exp = create_gbuffer(resolution);
			if(!gbuffer_exp)
				return {std::move(gbuffer_exp.error())};

			auto [u_renderpass_res, u_renderpass] = create_renderpass(gbuffer_exp.value(),
																	  evaluation_image_format);

			if(u_renderpass_res != vk::Result::eSuccess)
				return {UnexpectedVkResult(u_renderpass_res)};

			auto image_views_exp = create_framebuffer_image_views(gbuffer_exp.value(),
																  evaluation_image,
																  evaluation_image_format);

			hrs::scoped_call image_views_dtor([&image_views_exp, this]()
			{
				for(auto image_view : image_views_exp.value())
					parent_device->GetDevice().destroy(image_view);
			});

			if(!image_views_exp)
				return {UnexpectedVkResult(image_views_exp.error())};

			auto [u_framebuffer_res, u_framebuffer] = create_framebuffer(u_renderpass.get(),
																		 image_views_exp.value(),
																		 resolution);

			if(u_framebuffer_res != vk::Result::eSuccess)
				return {UnexpectedVkResult(u_framebuffer_res)};

			attachment_formats = create_attachment_formats(gbuffer_exp.value(), evaluation_image_format);

			auto descriptor_sets_data = create_descriptor_sets(image_views_exp.value(), frame_count);
			if(!descriptor_sets_data)
				return UnexpectedVkResult(descriptor_sets_data.error());

			auto [u_pool, u_descriptor_set_layout, _descriptor_sets] = std::move(descriptor_sets_data.value());
			for(auto &binding : shader_bindings)
				for(auto &deffered_shader : binding)
				{
					const auto &shader_modules = deffered_shader.shader_modules;
					std::uint32_t subpass = deffered_shader.subpass;
					auto shader_recreate_unexpected_res =
						deffered_shader.shader->Recreate(shader_modules,
														  u_renderpass.get(),
														  subpass,
														  globals_layout/*globals*/,
														  u_descriptor_set_layout.get()/*renderpass*/,
														  resolution);

					if(shader_recreate_unexpected_res)
						return shader_recreate_unexpected_res;
				}

			image_views_dtor.Drop();

			init(std::move(gbuffer_exp.value()),
				 u_renderpass.release(),
				 u_framebuffer.release(),
				 std::move(image_views_exp.value()),
				 attachment_formats,
				 u_pool.release(),
				 u_descriptor_set_layout.release(),
				 std::move(_descriptor_sets));
		}

		return {};
	}

	void DefferedPass::Render(vk::CommandBuffer command_buffer,
							  std::uint32_t frame_index,
							  vk::DescriptorSet globals_set,
							  const ComputedCamera &camera,
							  const Scene *scene)
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

		for(std::size_t subpass_index = 0; subpass_index < SubpassIndices::LastUnusedSubpassIndex; subpass_index++)
		{
			if(subpass_index != 0)
				command_buffer.nextSubpass(vk::SubpassContents::eInline);

			vk::DescriptorSet target_set = descriptor_sets[frame_index];

			scene->Render(command_buffer,
						  renderpass,
						  frame_index,
						  globals_set,
						  target_set,
						  subpass_index,
						  camera);
		}

		command_buffer.endRenderPass();
	}

	bool DefferedPass::IsCreated() const noexcept
	{
		return renderpass;
	}

	const DefferedPass::AttachmentFormatsArray & DefferedPass::GetAttachmentFormats() const noexcept
	{
		return attachment_formats;
	}

	hrs::unexpected_result DefferedPass::AddShader(SubpassIndices subpass_index,
												   DefferedPassShader *shader,
												   const std::map<vk::ShaderStageFlagBits, ShaderInfo> shader_infos)
	{
		ShaderInfoNode<DefferedPassShader> shader_info_node{shader_infos,
															subpass_index,
															shader};

		if(IsCreated())
		{
			auto shader_unexpected_res = shader->Recreate(shader_infos,
														  renderpass,
														  subpass_index,
														  VK_NULL_HANDLE,
														  descriptor_set_layout,
														  gbuffer.GetResolution());

			if(shader_unexpected_res)
				return shader_unexpected_res;
		}

		shader_bindings[subpass_index].push_back(std::move(shader_info_node));

		return {};
//#error START FROM HERE!!!!!!
	}

	hrs::expected<GBuffer, hrs::unexpected_result>
	DefferedPass::create_gbuffer(const vk::Extent2D &resolution)
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
			vk::Format::eD32Sfloat,//
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD16UnormS8Uint,
			vk::Format::eX8D24UnormPack32,
			vk::Format::eD16Unorm
		};

		vk::PhysicalDevice ph_device = parent_device->GetPhysicalDevice();

		std::array<vk::FormatFeatureFlags, GBuffer::BufferIndices::LastUnusedBufferIndex> formats_features;
		formats_features[GBuffer::BufferIndices::ColorBuffer] =
			vk::FormatFeatureFlagBits::eColorAttachment;
		formats_features[GBuffer::BufferIndices::DepthStencilBuffer] =
			vk::FormatFeatureFlagBits::eDepthStencilAttachment;

		std::array<vk::ImageUsageFlags, GBuffer::BufferIndices::LastUnusedBufferIndex> formats_usage;
		formats_usage[GBuffer::BufferIndices::ColorBuffer] =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eInputAttachment;
		formats_usage[GBuffer::BufferIndices::DepthStencilBuffer] =
			vk::ImageUsageFlagBits::eDepthStencilAttachment |
			vk::ImageUsageFlagBits::eInputAttachment;

		std::array<vk::Format, GBuffer::BufferIndices::LastUnusedBufferIndex> choosed_formats;


		auto find_best_format = [ph_device, &formats_features, &formats_usage, &resolution, &choosed_formats]
			<std::size_t N>(const std::array<vk::Format, N> &formats, std::size_t index) -> vk::Result
		{
			for(auto format : formats)
			{
				auto satisfy_res = IsFormatSatisfyRequirements(ph_device,
															   format,
															   formats_features[index],
															   vk::ImageType::e2D,
															   vk::ImageTiling::eOptimal,
															   formats_usage[index],
															   vk::Extent3D(resolution, 1),
															   1,
															   1,
															   vk::SampleCountFlagBits::e1,
															   {});

				if(satisfy_res == vk::Result::eSuccess)
				{
					choosed_formats[index] = format;
					return vk::Result::eSuccess;
				}
				else if(satisfy_res != vk::Result::eErrorFormatNotSupported)
					return satisfy_res;
			}

			return vk::Result::eErrorFormatNotSupported;
		};

		auto satisfy_res = find_best_format(color_buffer_formats, GBuffer::BufferIndices::ColorBuffer);
		if(satisfy_res != vk::Result::eSuccess)
			return {UnexpectedVkResult(satisfy_res)};

		satisfy_res = find_best_format(depth_stencil_buffer_formats, GBuffer::BufferIndices::DepthStencilBuffer);
		if(satisfy_res != vk::Result::eSuccess)
			return {UnexpectedVkResult(satisfy_res)};

		GBufferImageParams color_buffer_params(formats_usage[GBuffer::BufferIndices::ColorBuffer],
											   choosed_formats[GBuffer::BufferIndices::ColorBuffer],
											   vk::ImageLayout::eUndefined);

		GBufferImageParams depth_stencil_buffer_params(formats_usage[GBuffer::BufferIndices::DepthStencilBuffer],
													   choosed_formats[GBuffer::BufferIndices::DepthStencilBuffer],
													   vk::ImageLayout::eUndefined);

		GBuffer gbuffer(parent_device);
		auto unexpected_res = gbuffer.Recreate(color_buffer_params,
											   depth_stencil_buffer_params,
											   resolution);

		if(unexpected_res)
			return unexpected_res;

		return gbuffer;
	}

	vk::ResultValue<vk::UniqueRenderPass>
	DefferedPass::create_renderpass(const GBuffer &_gbuffer,
									vk::Format evaluation_image_format) noexcept
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
									  evaluation_image_format,
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
									vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::AttachmentReference(AttachmentIndices::GBufferDepthStencil,
									vk::ImageLayout::eShaderReadOnlyOptimal)
		};

		const std::array evaluation_subpass_color_attachments =
		{
			vk::AttachmentReference(AttachmentIndices::EvaluationImage,
									vk::ImageLayout::eColorAttachmentOptimal),
		};

		const std::array subpasses =
		{
			vk::SubpassDescription({},//rasterization subpass
								   vk::PipelineBindPoint::eGraphics,
								   {},
								   rasterization_subpass_color_attachments,
								   {},
								   rasterization_subpass_depth_stencil_attachments.data(),
								   {}),
			vk::SubpassDescription({},//evaluation subpass
								   vk::PipelineBindPoint::eGraphics,
								   evaluation_subpass_input_attachments,
								   evaluation_subpass_color_attachments,
								   {},
								   {},
								   {}),
		};

		const std::array subpass_dependencies =
		{
			vk::SubpassDependency(VK_SUBPASS_EXTERNAL,
								  0,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::PipelineStageFlagBits::eEarlyFragmentTests,
								  vk::AccessFlagBits::eColorAttachmentWrite,
								  vk::AccessFlagBits::eDepthStencilAttachmentRead,
								  vk::DependencyFlagBits::eByRegion),

			vk::SubpassDependency(0,
								  1,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::AccessFlagBits::eColorAttachmentWrite,
								  vk::AccessFlagBits::eColorAttachmentRead,
								  vk::DependencyFlagBits::eByRegion),

			vk::SubpassDependency(1,
								  VK_SUBPASS_EXTERNAL,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::AccessFlagBits::eColorAttachmentWrite,
								  vk::AccessFlagBits::eColorAttachmentRead,
								  vk::DependencyFlagBits::eByRegion)
		};

		vk::RenderPassCreateInfo info({},
									  attachments,
									  subpasses,
									  subpass_dependencies);

		return parent_device->GetDevice().createRenderPassUnique(info);
	}

	hrs::expected<DefferedPass::FramebufferImageViewsArray, vk::Result>
	DefferedPass::create_framebuffer_image_views(const GBuffer &_gbuffer,
												 vk::Image evaluation_image,
												 vk::Format evaluation_image_format)

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
		hrs::scoped_call views_dtor([&_views, this]()
		{
			for(auto &view : _views)
				parent_device->GetDevice().destroy(view);
		});

		auto [color_view_res, color_view] = parent_device->GetDevice().createImageView(info);
		if(color_view_res != vk::Result::eSuccess)
			return color_view_res;

		_views[AttachmentIndices::GBufferColor] = color_view;

		info.image = _gbuffer.GetBuffer(GBuffer::BufferIndices::DepthStencilBuffer).image.image;
		info.format = _gbuffer.GetBuffer(GBuffer::BufferIndices::DepthStencilBuffer).format;
		info.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth |
															  (IsDepthStencilFormat(info.format) ?
															  vk::ImageAspectFlagBits::eStencil :
															  vk::ImageAspectFlagBits{}),
														  0,
														  1,
														  0,
														  1);

		auto [depth_view_res, depth_view] = parent_device->GetDevice().createImageView(info);
		if(depth_view_res != vk::Result::eSuccess)
			return depth_view_res;

		_views[AttachmentIndices::GBufferDepthStencil] = depth_view;

		info.image = evaluation_image;
		info.format = evaluation_image_format;
		info.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor,
														  0,
														  1,
														  0,
														  1);

		auto [eval_view_res, eval_view] = parent_device->GetDevice().createImageView(info);
		if(eval_view_res != vk::Result::eSuccess)
			return eval_view_res;

		_views[AttachmentIndices::EvaluationImage] = eval_view;

		views_dtor.Drop();
		return _views;
	}

	vk::ResultValue<vk::UniqueFramebuffer>
	DefferedPass::create_framebuffer(vk::RenderPass rpass,
									 const FramebufferImageViewsArray &image_views,
									 const vk::Extent2D &resolution) noexcept
	{
		vk::FramebufferCreateInfo info({},
									   rpass,
									   image_views,
									   resolution.width,
									   resolution.height,
									   1);

		return parent_device->GetDevice().createFramebufferUnique(info);
	}

	DefferedPass::AttachmentFormatsArray
	DefferedPass::create_attachment_formats(const GBuffer &_gbuffer,
											vk::Format evaluation_image_format)
	{
		AttachmentFormatsArray formats;
		formats[AttachmentIndices::GBufferColor] = _gbuffer.GetBuffer(GBuffer::BufferIndices::ColorBuffer).format;
		formats[AttachmentIndices::GBufferDepthStencil] =
			_gbuffer.GetBuffer(GBuffer::BufferIndices::DepthStencilBuffer).format;
		formats[AttachmentIndices::EvaluationImage] = evaluation_image_format;

		return formats;
	}

	DefferedPass::create_descriptor_sets_result_t
	DefferedPass::create_descriptor_sets(const FramebufferImageViewsArray &views,
										 std::uint32_t frame_count)
	{
		vk::Device device_handle = parent_device->GetDevice();
		const std::array pool_sizes =
		{
			vk::DescriptorPoolSize(vk::DescriptorType::eInputAttachment, 2 * frame_count)
		};

		const vk::DescriptorPoolCreateInfo pool_info({},
													 frame_count,
													 pool_sizes);

		auto [u_pool_res, u_pool] = device_handle.createDescriptorPoolUnique(pool_info);
		if(u_pool_res != vk::Result::eSuccess)
			return u_pool_res;

		const std::array bindings =
		{
			vk::DescriptorSetLayoutBinding(0,//color buffer input
										   vk::DescriptorType::eInputAttachment,
										   1,
										   vk::ShaderStageFlagBits::eFragment,
										   {}),
			vk::DescriptorSetLayoutBinding(1,//depth buffer input
										   vk::DescriptorType::eInputAttachment,
										   1,
										   vk::ShaderStageFlagBits::eFragment,
										   {}),
		};

		const vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_info({},
																		   bindings);

		auto [u_layout_res, u_layout] = device_handle.createDescriptorSetLayoutUnique(descriptor_set_layout_info);
		if(u_layout_res != vk::Result::eSuccess)
			return u_layout_res;

		const std::vector<vk::DescriptorSetLayout> descriptor_layouts(frame_count, u_layout.get());
		const vk::DescriptorSetAllocateInfo set_allocate_info(u_pool.get(), descriptor_layouts);

		auto [_descriptor_sets_res, _descriptor_sets] = device_handle.allocateDescriptorSets(set_allocate_info);
		if(_descriptor_sets_res != vk::Result::eSuccess)
			return _descriptor_sets_res;

		for(auto set : _descriptor_sets)
		{
			const vk::DescriptorImageInfo color_buffer_info({},
															views[AttachmentIndices::GBufferColor],
															vk::ImageLayout::eShaderReadOnlyOptimal);
			const vk::DescriptorImageInfo depth_stencil_buffer_info({},
																	views[AttachmentIndices::GBufferDepthStencil],
																	vk::ImageLayout::eShaderReadOnlyOptimal);

			const std::array writes =
			{
				vk::WriteDescriptorSet(set,//color bufer
									   0,
									   0,
									   vk::DescriptorType::eInputAttachment,
									   color_buffer_info),
				vk::WriteDescriptorSet(set,//depth stencil buffer
									   1,
									   0,
									   vk::DescriptorType::eInputAttachment,
									   depth_stencil_buffer_info)
			};

			device_handle.updateDescriptorSets(writes, {});
		}

		return std::tuple(std::move(u_pool), std::move(u_layout), std::move(_descriptor_sets));
	}
};
