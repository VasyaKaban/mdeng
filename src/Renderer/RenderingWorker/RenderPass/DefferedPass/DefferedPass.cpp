#include "DefferedPass.h"
#include "../../RenderingWorker.h"
#include "../../../../hrs/scoped_call.hpp"
#include "../../../../hrs/iterator_for_each.hpp"
#include "../../../../Vulkan/VulkanUtils.hpp"
#include "../../../../Vulkan/UnexpectedVkResult.hpp"
#include "../../../../Vulkan/VulkanFormatUtils.hpp"

namespace FireLand
{
	void DefferedPass::init(vk::RenderPass _renderpass,
							vk::Framebuffer _framebuffer,
							const FramebufferImageViewsArray &_framebuffer_image_views,
							vk::DescriptorPool _descriptor_pool,
							vk::DescriptorSetLayout _descriptor_set_layout,
							std::vector<vk::DescriptorSet> &&_descriptor_sets,
							const vk::Extent2D &_resolution) noexcept
	{
		renderpass = _renderpass;
		framebuffer = _framebuffer;
		framebuffer_image_views = _framebuffer_image_views;
		descriptor_pool = _descriptor_pool;
		descriptor_set_layout = _descriptor_set_layout;
		descriptor_sets = std::move(_descriptor_sets);
		resolution = _resolution;
	}

	void DefferedPass::init(vk::RenderPass _renderpass,
							vk::Framebuffer _framebuffer,
							const FramebufferImageViewsArray &_framebuffer_image_views,
							const vk::Extent2D &_resolution) noexcept
	{
		renderpass = _renderpass;
		framebuffer = _framebuffer;
		framebuffer_image_views = _framebuffer_image_views;
		resolution = _resolution;
	}

	DefferedPass::DefferedPass(RenderingWorker *_parent_worker) noexcept
		: parent_worker(_parent_worker)
	{
		hrs::assert_true_debug(_parent_worker, "Parent worker pointer points to null!");
		hrs::assert_true_debug(_parent_worker->IsCreated(), "Parent worker isn't created yet!");
	}

	DefferedPass::~DefferedPass()
	{
		Destroy();
	}

	DefferedPass::DefferedPass(DefferedPass &&rpass) noexcept
		: parent_worker(rpass.parent_worker),
		  renderpass(std::exchange(rpass.renderpass, VK_NULL_HANDLE)),
		  framebuffer(std::exchange(rpass.framebuffer, VK_NULL_HANDLE)),
		  framebuffer_image_views(rpass.framebuffer_image_views),
		  descriptor_pool(std::exchange(rpass.descriptor_pool, VK_NULL_HANDLE)),
		  descriptor_set_layout(std::exchange(rpass.descriptor_set_layout, VK_NULL_HANDLE)),
		  descriptor_sets(std::move(rpass.descriptor_sets)),
		  shader_bindings(std::move(rpass.shader_bindings)),
		  resolution(rpass.resolution) {}

	DefferedPass & DefferedPass::operator=(DefferedPass &&rpass) noexcept
	{
		Destroy();

		parent_worker = rpass.parent_worker;
		renderpass = std::exchange(rpass.renderpass, VK_NULL_HANDLE);
		framebuffer = std::exchange(rpass.framebuffer, VK_NULL_HANDLE);
		framebuffer_image_views = rpass.framebuffer_image_views;
		descriptor_pool = std::exchange(rpass.descriptor_pool, VK_NULL_HANDLE);
		descriptor_set_layout = std::exchange(rpass.descriptor_set_layout, VK_NULL_HANDLE);
		descriptor_sets = std::move(rpass.descriptor_sets);
		shader_bindings = std::move(rpass.shader_bindings);
		resolution = rpass.resolution;

		return *this;
	}

	void DefferedPass::Destroy()
	{
		destroy_shader_bindings();
		destroy_descriptor_sets();
		destroy_renderpass_and_framebuffer();
	}

	hrs::unexpected_result DefferedPass::Recreate(const vk::Extent2D &_resolution, std::uint32_t frame_count)
	{
		destroy_renderpass_and_framebuffer();
		bool recreate_full = false;
		if(IsBadExtent(resolution))
		{
			//simply destroy without shaders and descriptors
			return {};
		}
		else
		{
			if(IsCreated())//only if descriptor_pool
			{
				if(frame_count == descriptor_sets.size())
				{
					//recreate all without descriptors
					//write descriptors
					auto [u_renderpass_res, u_renderpass] = create_renderpass();
					if(u_renderpass_res != vk::Result::eSuccess)
						return UnexpectedVkResult(u_renderpass_res);

					auto framebuffer_exp = create_framebuffer(u_renderpass.get(), _resolution);
					if(!framebuffer_exp)
						return UnexpectedVkResult(framebuffer_exp.error());

					auto [u_framebuffer, _framebuffer_image_views] = std::move(framebuffer_exp.value());

					write_descriptor_sets(_framebuffer_image_views, descriptor_sets);

					init(u_renderpass.release(),
						 u_framebuffer.release(),
						 _framebuffer_image_views,
						 _resolution);
				}
				else
				{
					//recreate full
					destroy_descriptor_sets();
					recreate_full = true;
				}
			}
			else
			{
				//simply create new
				recreate_full = true;
			}

			if(recreate_full)
			{
				auto [u_renderpass_res, u_renderpass] = create_renderpass();
				if(u_renderpass_res != vk::Result::eSuccess)
					return UnexpectedVkResult(u_renderpass_res);

				auto framebuffer_exp = create_framebuffer(u_renderpass.get(), _resolution);
				if(!framebuffer_exp)
					return UnexpectedVkResult(framebuffer_exp.error());

				auto [u_framebuffer, _framebuffer_image_views] = std::move(framebuffer_exp.value());

				hrs::scoped_call image_views_dtor([this, &_framebuffer_image_views]()
				{
					vk::Device device_handle = parent_worker->GetDevice()->GetHandle();
					for(const auto img_view : _framebuffer_image_views)
						device_handle.destroy(img_view);
				});

				auto descriptors_exp = create_descriptor_sets(framebuffer_image_views, frame_count);
				if(!descriptors_exp)
					return UnexpectedVkResult(descriptors_exp.error());

				auto [u_descriptor_pool, u_descriptor_layout, _sets] = std::move(descriptors_exp.value());

				image_views_dtor.Drop();
				init(u_renderpass.release(),
					 u_framebuffer.release(),
					 _framebuffer_image_views,
					 u_descriptor_pool.release(),
					 u_descriptor_layout.release(),
					 std::move(_sets),
					 _resolution);
			}

			return {};
		}

		vk::DescriptorSetLayout globals_set_layout = parent_worker->GetGlobalsDescriptorSetLayout();
		for(std::uint32_t i = 0; i != shader_bindings.size(); i++)
			for(auto &shader : shader_bindings[i])
			{
				const auto &shader_modules = shader.shader_modules;
				auto unexpected_res = shader.shader->Recreate(shader_modules,
															  renderpass,
															  i,
															  globals_set_layout,
															  descriptor_set_layout,
															  resolution);

				if(unexpected_res)
					return unexpected_res;
			}
	}

	void DefferedPass::Render(vk::CommandBuffer command_buffer,
							  std::uint32_t frame_index,
							  const RenderInputs &inputs)
	{
		hrs::assert_true_debug(IsCreated(), "DefferedPass isn't create yet!");
		std::array<vk::ClearValue, AttachmentIndices::LastUnusedAttachmentIndex> clear_values;
		clear_values[AttachmentIndices::GBufferColor] = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f);
		clear_values[AttachmentIndices::GBufferDepthStencil] = vk::ClearDepthStencilValue(1.0f, 0);
		clear_values[AttachmentIndices::GBufferNormals] = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f);
		clear_values[AttachmentIndices::EvaluationImage] = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f);

		vk::RenderPassBeginInfo begin_info(renderpass,
										   framebuffer,
										   vk::Rect2D(0, resolution),
										   clear_values);
		command_buffer.beginRenderPass(begin_info, vk::SubpassContents::eInline);

		vk::DescriptorSet globals_set = parent_worker->GetGlobalsDescriptorSet();

		for(std::size_t subpass_index = 0; subpass_index < SubpassIndices::LastUnusedSubpassIndex; subpass_index++)
		{
			if(subpass_index != 0)
				command_buffer.nextSubpass(vk::SubpassContents::eInline);

			vk::DescriptorSet target_set = descriptor_sets[frame_index];

			inputs.scene->Render(command_buffer,
								 renderpass,
								 frame_index,
								 globals_set,
								 target_set,
								 subpass_index,
								 inputs);
		}

		command_buffer.endRenderPass();
	}

	bool DefferedPass::IsCreated() const noexcept
	{
		return descriptor_pool;
	}

	hrs::unexpected_result
	DefferedPass::AddShader(SubpassIndices subpass_index,
							DefferedPassShader *shader,
							const std::map<vk::ShaderStageFlagBits, ShaderInfo> &shader_infos)
	{
		hrs::assert_true_debug(shader, "Shader pointer points to null!");

		if(IsCreated())
		{
			auto unexpected_res = shader->Recreate(shader_infos,
												   renderpass,
												   subpass_index,
												   parent_worker->GetGlobalsDescriptorSetLayout(),
												   descriptor_set_layout,
												   resolution);

			if(unexpected_res)
				return unexpected_res;
		}

		shader_bindings[subpass_index].emplace_back(shader_infos, shader);

		return {};
	}

	const  DefferedPass::ShaderSubpassBindingsArray & DefferedPass::GetShaderSubpassBindingsArray() const noexcept
	{
		return shader_bindings;
	}

	const DefferedPass::SubpassShaders &
	DefferedPass::GetSubpassShaders(SubpassIndices subpass_index) const noexcept
	{
		return shader_bindings[subpass_index];
	}

	void DefferedPass::DropShader(SubpassIndices subpass_index, SubpassShaders::const_iterator it) noexcept
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(shader_bindings[subpass_index], it),
							   "Shader is not a part of this renderpass!");

		shader_bindings[subpass_index].erase(it);
	}

	void DefferedPass::DropShader(SubpassIndices subpass_index, const DefferedPassShader *shader) noexcept
	{
		hrs::iterator_for_each(shader_bindings[subpass_index],
							   [shader, subpass_index, this](SubpassShaders::const_iterator it) -> bool
		{
			if(it->shader.get() == shader)
			{
				shader_bindings[subpass_index].erase(it);
				return true;
			}

			return false;
		});
	}

	void DefferedPass::destroy_renderpass_and_framebuffer() noexcept
	{
		if(!IsCreated())
			return;

		vk::Device device_handle = parent_worker->GetDevice()->GetHandle();
		framebuffer = (device_handle.destroy(framebuffer), VK_NULL_HANDLE);
		renderpass = (device_handle.destroy(renderpass), VK_NULL_HANDLE);
	}

	void DefferedPass::destroy_descriptor_sets() noexcept
	{
		if(!IsCreated())
			return;

		vk::Device device_handle = parent_worker->GetDevice()->GetHandle();
		descriptor_pool = (device_handle.resetDescriptorPool(descriptor_pool),
						   device_handle.destroy(descriptor_pool),
						   VK_NULL_HANDLE);
		descriptor_set_layout = (device_handle.destroy(descriptor_set_layout), VK_NULL_HANDLE);
		descriptor_sets.clear();
	}

	void DefferedPass::destroy_shader_bindings() noexcept
	{
		for(auto &binding : shader_bindings)
			binding.clear();
	}

	vk::ResultValue<vk::UniqueRenderPass>
	DefferedPass::create_renderpass() noexcept
	{
		const auto *gbuffer = parent_worker->GetGBuffer();
		const auto evaluation_image = parent_worker->GetDefferedPassEvaluationImage();
		const std::array attachments =
		{
			vk::AttachmentDescription({},
									  gbuffer->GetBuffer(GBuffer::BufferIndices::ColorBuffer).format,
									  vk::SampleCountFlagBits::e1,
									  vk::AttachmentLoadOp::eClear,
									  vk::AttachmentStoreOp::eStore,
									  vk::AttachmentLoadOp::eDontCare,
									  vk::AttachmentStoreOp::eDontCare,
									  vk::ImageLayout::eUndefined,
									  vk::ImageLayout::eColorAttachmentOptimal),
			vk::AttachmentDescription({},
									  gbuffer->GetBuffer(GBuffer::BufferIndices::DepthStencilBuffer).format,
									  vk::SampleCountFlagBits::e1,
									  vk::AttachmentLoadOp::eClear,
									  vk::AttachmentStoreOp::eStore,
									  vk::AttachmentLoadOp::eDontCare,//stencil!
									  vk::AttachmentStoreOp::eDontCare,//stencil!!
									  vk::ImageLayout::eUndefined,
									  vk::ImageLayout::eColorAttachmentOptimal),
			vk::AttachmentDescription({},
									  gbuffer->GetBuffer(GBuffer::BufferIndices::NormalsBuffer).format,
									  vk::SampleCountFlagBits::e1,
									  vk::AttachmentLoadOp::eClear,
									  vk::AttachmentStoreOp::eStore,
									  vk::AttachmentLoadOp::eDontCare,
									  vk::AttachmentStoreOp::eDontCare,
									  vk::ImageLayout::eUndefined,
									  vk::ImageLayout::eColorAttachmentOptimal),
			vk::AttachmentDescription({},
									  evaluation_image.format,
									  vk::SampleCountFlagBits::e1,
									  vk::AttachmentLoadOp::eClear,
									  vk::AttachmentStoreOp::eStore,
									  vk::AttachmentLoadOp::eDontCare,
									  vk::AttachmentStoreOp::eDontCare,
									  vk::ImageLayout::eUndefined,
									  vk::ImageLayout::eColorAttachmentOptimal),
		};

		const std::array geometry_pass_color_attachments =
		{
			vk::AttachmentReference(AttachmentIndices::GBufferColor,
									vk::ImageLayout::eColorAttachmentOptimal),
			vk::AttachmentReference(AttachmentIndices::GBufferNormals,
									vk::ImageLayout::eColorAttachmentOptimal),
		};

		const std::array geometry_pass_depth_stencil_attachment =
		{
			vk::AttachmentReference(AttachmentIndices::GBufferDepthStencil,
									vk::ImageLayout::eDepthStencilAttachmentOptimal),
		};

		const std::array light_pass_input_attachments =
		{
			vk::AttachmentReference(AttachmentIndices::GBufferColor,
									vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::AttachmentReference(AttachmentIndices::GBufferDepthStencil,
									vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::AttachmentReference(AttachmentIndices::GBufferNormals,
									vk::ImageLayout::eShaderReadOnlyOptimal),
		};

		const std::array light_pass_color_attachment =
		{
			vk::AttachmentReference(AttachmentIndices::EvaluationImage,
									vk::ImageLayout::eColorAttachmentOptimal),
		};

		const std::array subpasses =
		{
			vk::SubpassDescription({},
								   vk::PipelineBindPoint::eGraphics,
								   {},
								   geometry_pass_color_attachments,
								   {},
								   geometry_pass_depth_stencil_attachment.data(),
								   {}),
			vk::SubpassDescription({},
								   vk::PipelineBindPoint::eGraphics,
								   light_pass_input_attachments,
								   light_pass_color_attachment,
								   {},
								   {},
								   {}),
		};

		const std::array subpass_dependencies =
		{
			vk::SubpassDependency(VK_SUBPASS_EXTERNAL,
								  SubpassIndices::GeometrySubpassIndex,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::PipelineStageFlagBits::eEarlyFragmentTests,
								  vk::AccessFlagBits::eColorAttachmentWrite |
								  vk::AccessFlagBits::eDepthStencilAttachmentWrite,
								  vk::AccessFlagBits::eColorAttachmentRead |
								  vk::AccessFlagBits::eColorAttachmentWrite |
								  vk::AccessFlagBits::eDepthStencilAttachmentRead |
								  vk::AccessFlagBits::eDepthStencilAttachmentWrite,
								  vk::DependencyFlagBits::eByRegion),
			vk::SubpassDependency(SubpassIndices::GeometrySubpassIndex,
								  SubpassIndices::LightSubpassIndex,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::AccessFlagBits::eColorAttachmentWrite |
								  vk::AccessFlagBits::eDepthStencilAttachmentWrite,
								  vk::AccessFlagBits::eInputAttachmentRead,
								  vk::DependencyFlagBits::eByRegion),
			vk::SubpassDependency(SubpassIndices::LightSubpassIndex,
								  VK_SUBPASS_EXTERNAL,
								  vk::PipelineStageFlagBits::eColorAttachmentOutput,
								  vk::PipelineStageFlagBits::eTopOfPipe,
								  vk::AccessFlagBits::eColorAttachmentWrite,
								  vk::AccessFlagBits::eColorAttachmentRead |
								  vk::AccessFlagBits::eColorAttachmentWrite,
								  vk::DependencyFlagBits::eByRegion),
		};

		const vk::RenderPassCreateInfo info({},
											attachments,
											subpasses,
											subpass_dependencies);

		return parent_worker->GetDevice()->GetHandle().createRenderPassUnique(info);
	}

	hrs::expected<DefferedPass::create_framebuffer_result_t, vk::Result>
	DefferedPass::create_framebuffer(vk::RenderPass rpass, const vk::Extent2D &_resolution) noexcept
	{
		vk::Device device_handle = parent_worker->GetDevice()->GetHandle();
		FramebufferImageViewsArray image_views;
		hrs::scoped_call image_views_dtor([&image_views, this, device_handle]()
		{
			for(const auto &view : image_views)
				device_handle.destroy(view);
		});
		vk::ImageViewCreateInfo image_view_info({},
												{/*image*/},
												vk::ImageViewType::e2D,
												{/*format*/},
												vk::ComponentMapping(/*all identity*/),
												vk::ImageSubresourceRange({/*image aspect*/},
																		  0,
																		  1,
																		  0,
																		  1));

		const auto *gbuffer = parent_worker->GetGBuffer();
		const auto evaluation_image = parent_worker->GetDefferedPassEvaluationImage();
		auto fill_info_and_create_image_view = [&](AttachmentIndices index)
		{
			ImageFormat img_format;
			if(index < AttachmentIndices::GBufferImagesBorder)
			{
#warning BE CAREFUL WITH INDEX!!!!!!!!!!!!
				img_format =
				{gbuffer->GetBufferImage(static_cast<GBuffer::BufferIndices>(index)),
				gbuffer->GetBuffer(static_cast<GBuffer::BufferIndices>(index)).format};
			}
			else
				img_format = evaluation_image;

			image_view_info.setImage(img_format.image).setFormat(img_format.format);
			if(index == AttachmentIndices::GBufferDepthStencil)
			{
				vk::ImageAspectFlags depth_stencil_aspect = vk::ImageAspectFlagBits::eDepth;
				if(IsDepthStencilFormat(gbuffer->GetBuffer(GBuffer::BufferIndices::DepthStencilBuffer).format))
					depth_stencil_aspect |= vk::ImageAspectFlagBits::eStencil;

				image_view_info.subresourceRange.setAspectMask(depth_stencil_aspect);
			}
			else
				image_view_info.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);

			return device_handle.createImageView(image_view_info);
		};

		for(std::size_t i = AttachmentIndices::GBufferColor; i != AttachmentIndices::LastUnusedAttachmentIndex; i++)
		{
			auto [image_view_res, image_view] = fill_info_and_create_image_view(AttachmentIndices(i));
			if(image_view_res != vk::Result::eSuccess)
				return image_view_res;

			image_views[i] = image_view;
		}

		const vk::FramebufferCreateInfo info({},
											 rpass,
											 image_views,
											 _resolution.width,
											 _resolution.height,
											 1);

		auto [u_framebuffer_res, u_framebuffer] = device_handle.createFramebufferUnique(info);
		if(u_framebuffer_res != vk::Result::eSuccess)
			return u_framebuffer_res;

		image_views_dtor.Drop();
		return std::tuple{std::move(u_framebuffer), image_views};
	}

	hrs::expected<DefferedPass::create_descriptor_sets_result_t, vk::Result>
	DefferedPass::create_descriptor_sets(const FramebufferImageViewsArray &views, std::uint32_t frame_count)
	{
		vk::Device device_handle = parent_worker->GetDevice()->GetHandle();
		const std::array pool_sizes =
		{
			vk::DescriptorPoolSize(vk::DescriptorType::eInputAttachment,
								   AttachmentIndices::GBufferImagesBorder * frame_count)
		};

		const vk::DescriptorPoolCreateInfo pool_info({},
													 frame_count,
													 pool_sizes);

		auto [u_descriptor_pool_res, u_descriptor_pool] = device_handle.createDescriptorPoolUnique(pool_info);
		if(u_descriptor_pool_res != vk::Result::eSuccess)
			return u_descriptor_pool_res;

		const std::array descriptor_set_layout_bindings =
		{
			vk::DescriptorSetLayoutBinding(AttachmentIndices::GBufferColor,
										   vk::DescriptorType::eInputAttachment,
										   1,
										   vk::ShaderStageFlagBits::eFragment,
										   {}),
			vk::DescriptorSetLayoutBinding(AttachmentIndices::GBufferDepthStencil,
										   vk::DescriptorType::eInputAttachment,
										   1,
										   vk::ShaderStageFlagBits::eFragment,
										   {}),
			vk::DescriptorSetLayoutBinding(AttachmentIndices::GBufferNormals,
										   vk::DescriptorType::eInputAttachment,
										   1,
										   vk::ShaderStageFlagBits::eFragment,
										   {})
		};

		const vk::DescriptorSetLayoutCreateInfo set_layout_info({},
																descriptor_set_layout_bindings);

		auto [u_set_layout_res, u_set_layout] = device_handle.createDescriptorSetLayoutUnique(set_layout_info);
		if(u_set_layout_res != vk::Result::eSuccess)
			return u_set_layout_res;

		const std::vector set_layouts(frame_count, u_set_layout.get());
		const vk::DescriptorSetAllocateInfo allocate_info(u_descriptor_pool.get(),
														  set_layouts);

		auto [_sets_res, _sets] = device_handle.allocateDescriptorSets(allocate_info);
		if(_sets_res != vk::Result::eSuccess)
			return _sets_res;

		write_descriptor_sets(views, _sets);

		return std::tuple{std::move(u_descriptor_pool), std::move(u_set_layout), std::move(_sets)};
	}

	void DefferedPass::write_descriptor_sets(const FramebufferImageViewsArray &views,
											 const std::vector<vk::DescriptorSet> &sets) const noexcept
	{
		const vk::DescriptorImageInfo
			color_buffer_image_info({},
									views[AttachmentIndices::GBufferColor],
									vk::ImageLayout::eShaderReadOnlyOptimal);

		const vk::DescriptorImageInfo
			depth_stencil_buffer_image_info({},
											views[AttachmentIndices::GBufferDepthStencil],
											vk::ImageLayout::eShaderReadOnlyOptimal);

		const vk::DescriptorImageInfo
			normals_buffer_image_info({},
									  views[AttachmentIndices::GBufferNormals],
									  vk::ImageLayout::eShaderReadOnlyOptimal);
		std::array descriptor_writes =
		{

			vk::WriteDescriptorSet({/*set*/},
								   AttachmentIndices::GBufferColor,
								   0,
								   1,
								   vk::DescriptorType::eInputAttachment,
								   &color_buffer_image_info,
								   {}, {}),
			vk::WriteDescriptorSet({/*set*/},
								   AttachmentIndices::GBufferDepthStencil,
								   0,
								   1,
								   vk::DescriptorType::eInputAttachment,
								   &depth_stencil_buffer_image_info,
								   {}, {}),
			vk::WriteDescriptorSet({/*set*/},
								   AttachmentIndices::GBufferNormals,
								   0,
								   1,
								   vk::DescriptorType::eInputAttachment,
								   &normals_buffer_image_info,
								   {}, {})
		};

		vk::Device device_handle = parent_worker->GetDevice()->GetHandle();

		for(auto set : sets)
		{
			for(auto &write : descriptor_writes)
				write.setDstSet(set);

			device_handle.updateDescriptorSets(descriptor_writes, {});
		}
	}

	///////////////////////////////////////////////////////

	/*hrs::expected<GBuffer, hrs::unexpected_result>
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
	}*/
};
