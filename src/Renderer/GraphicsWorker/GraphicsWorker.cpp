#include "GraphicsWorker.h"

namespace FireLand
{
	GraphicsWorker::GraphicsWorker(Device *_device,
								   const Surface *_surface,
								   Swapchain &&_swapchain,
								   ImageFormatBounds &&_renderpasses_output_image,
								   vk::Queue _render_queue,
								   std::uint32_t _queue_family_index,
								   vk::CommandPool _command_pool,
								   PerFrameResources &&_per_frame_resources)
		: device(_device),
		  surface(_surface),
		  swapchain(std::move(_swapchain)),
		  renderpasses_output_image(std::move(_renderpasses_output_image)),
		  render_queue(_render_queue),
		  queue_family_index(_queue_family_index),
		  command_pool(_command_pool),
		  per_frame_resources(std::move(_per_frame_resources)),
		  is_swapchain_needs_to_be_recreated(false) {}

	GraphicsWorker::~GraphicsWorker()
	{
		Destroy();
	}

	void GraphicsWorker::Destroy()
	{
		/*!!!!!! [[maybe_unused]]auto _ = render_queue.waitIdle();*/
		swapchain.Destroy();
		vk::Device device_handle = device->GetDevice();
		device->GetAllocator()->Destroy(renderpasses_output_image.image, false);
		per_frame_resources.Destroy(device_handle, command_pool);
		device_handle.destroy(command_pool);
		per_frame_resources.target_frame_index = 0;
	}

	GraphicsWorker::GraphicsWorker(GraphicsWorker &&worker) noexcept
		: device(worker.device),
		  surface(worker.surface),
		  swapchain(std::move(worker.swapchain)),
		  renderpasses_output_image(std::move(worker.renderpasses_output_image)),
		  render_queue(worker.render_queue),
		  queue_family_index(worker.queue_family_index),
		  command_pool(worker.command_pool),
		  per_frame_resources(std::move(worker.per_frame_resources)),
		  is_swapchain_needs_to_be_recreated(worker.is_swapchain_needs_to_be_recreated)
	{
		worker.command_pool = VK_NULL_HANDLE;
		worker.per_frame_resources.target_frame_index = 0;
		worker.is_swapchain_needs_to_be_recreated = false;
	}

	hrs::expected<GraphicsWorker, GraphicsWorker::RecreateError>
	GraphicsWorker::Create(Device *_device,
						   const Surface *_surface,
						   std::uint32_t _queue_family_index,
						   std::uint32_t queue_index,
						   vk::Extent2D resolution,
						   vk::PresentModeKHR swapchain_present_mode,
						   std::uint32_t frames_count)
	{
		hrs::assert_true_debug(_device, "Parent device is points to null!");
		hrs::assert_true_debug(_surface, "Parent surface is points to null!");
		hrs::assert_true_debug(_device->GetDevice(), "Parent device isn't created yet!");
		hrs::assert_true_debug(_surface->GetSurface(), "Parent surface isn't created yet!");
		hrs::assert_true_debug(frames_count != 0, "Count of frames must be more than zero!");

		vk::Device device_handle = _device->GetDevice();
		vk::PhysicalDevice ph_device = _device->GetPhysicalDevice();
		vk::Queue _render_queue = device_handle.getQueue(_queue_family_index, queue_index);

		auto swapchain_exp = create_swapchain(_device,
											  _surface,
											  resolution,
											  swapchain_present_mode,
											  VK_NULL_HANDLE);

		if(!swapchain_exp)
			return {swapchain_exp.error()};

		const vk::CommandPoolCreateInfo pool_info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
												  vk::CommandPoolCreateFlagBits::eTransient,
												  _queue_family_index);

		auto [u_command_pool_res, u_command_pool] = device_handle.createCommandPoolUnique(pool_info);
		if(u_command_pool_res != vk::Result::eSuccess)
			return {u_command_pool_res};

		auto renderpasses_output_image_exp =
			create_renderpasses_output_image(_device,
											 swapchain_exp.value().GetSurfaceFormat().format,
											 resolution);

		if(!renderpasses_output_image_exp)
			return {renderpasses_output_image_exp.error()};

		auto per_frame_resources_exp = PerFrameResources::Create(device_handle,
																 u_command_pool.get(),
																 frames_count);

		if(!per_frame_resources_exp)
		{
			_device->GetAllocator()->Destroy(renderpasses_output_image_exp.value().image, false);
			return {per_frame_resources_exp.error()};
		}


		return GraphicsWorker(_device,
							  _surface,
							  std::move(swapchain_exp.value()),
							  std::move(renderpasses_output_image_exp.value()),
							  _render_queue,
							  _queue_family_index,
							  u_command_pool.release(),
							  std::move(per_frame_resources_exp.value()));
	}

	GraphicsWorker::RenderError GraphicsWorker::Render(const RenderInputs &inputs)
	{
		vk::Device device_handle = device->GetDevice();

		/*
		 * Success, timeout(no), not ready(no), suboptimal -> success op -> semaphore is signaled
		 * Error -> error op -> semaphore isn't signaled
		 */

		auto [acquire_res, acquire_index, is_fences_waited] = acquire_swapchain_image(inputs);
		if(acquire_res.keeps<GraphicsWorkerResult>())
			return acquire_res;

		switch(acquire_res.get<vk::Result>())
		{
			case vk::Result::eSuccess:
			case vk::Result::eSuboptimalKHR:
				break;
			default:
				return acquire_res;
				break;
		}

		vk::CommandBuffer &comm_buffer =
			per_frame_resources.command_buffers[per_frame_resources.target_frame_index];

		vk::Fence &fence = per_frame_resources.fences[per_frame_resources.target_frame_index];
		vk::Image acquired_image = swapchain.GetImage(acquire_index);
		vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		if(!is_fences_waited)
		{
			auto wait_res = device_handle.waitForFences(fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
			if(wait_res != vk::Result::eSuccess)
				return wait_res;
		}

		auto reset_res = device_handle.resetFences(fence);
		if(reset_res != vk::Result::eSuccess)
			return reset_res;

		auto begin_res = comm_buffer.begin(begin_info);
		if(begin_res != vk::Result::eSuccess)
			return begin_res;

		//render

		vk::ImageSubresourceRange layout_change_subres_range(vk::ImageAspectFlagBits::eColor,
															 0,
															 1,
															 0,
															 1);

		vk::ImageMemoryBarrier layout_change_barrier_to_optimal_dst({},
																	{},
																	vk::ImageLayout::eUndefined,
																	vk::ImageLayout::eTransferDstOptimal,
																	VK_QUEUE_FAMILY_IGNORED,
																	VK_QUEUE_FAMILY_IGNORED,
																	acquired_image,
																	layout_change_subres_range);


		comm_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eBottomOfPipe,
									vk::PipelineStageFlagBits::eTransfer,
									vk::DependencyFlagBits::eByRegion,
									{},
									{},
									layout_change_barrier_to_optimal_dst);


		const static vk::ClearColorValue clear_color(1.0f, 1.0f, 1.0f, 1.0f);
		comm_buffer.clearColorImage(acquired_image,
									vk::ImageLayout::eTransferDstOptimal,
									clear_color,
									layout_change_subres_range);

		vk::ImageMemoryBarrier layout_change_barrier({},
													 {},
													 vk::ImageLayout::eTransferDstOptimal,
													 vk::ImageLayout::ePresentSrcKHR,
													 VK_QUEUE_FAMILY_IGNORED,
													 VK_QUEUE_FAMILY_IGNORED,
													 acquired_image,
													 layout_change_subres_range);

		comm_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
									vk::PipelineStageFlagBits::eTopOfPipe,
									vk::DependencyFlagBits::eByRegion,
									{},
									{},
									layout_change_barrier);

		auto end_res = comm_buffer.end();
		if(end_res != vk::Result::eSuccess)
			return end_res;

		const vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eBottomOfPipe};
		const vk::Semaphore wait_semaphores[] = {swapchain.GetAcquireSignalSemaphore()};
		const vk::Semaphore signal_semaphores[] = {swapchain.GetPresentWaitSemaphore()};
		const vk::CommandBuffer command_buffers[] = {comm_buffer};
		vk::SubmitInfo submit_info(wait_semaphores,
								   wait_stages,
								   command_buffers,
								   signal_semaphores);

		auto submit_res = render_queue.submit(submit_info, fence);
		if(submit_res != vk::Result::eSuccess)
			return submit_res;

		per_frame_resources.UpdateTargetFrameIndex();
		return present_swapchain_image(acquire_index);
	}

	Device * GraphicsWorker::GetDevice() noexcept
	{
		return device;
	}

	const Device * GraphicsWorker::GetDevice() const noexcept
	{
		return device;
	}

	const Surface * GraphicsWorker::GetSurface() const noexcept
	{
		return surface;
	}

	const Swapchain & GraphicsWorker::GetSwapchain() const noexcept
	{
		return swapchain;
	}

	vk::Queue GraphicsWorker::GetRenderQueue() const noexcept
	{
		return render_queue;
	}

	std::uint32_t GraphicsWorker::GetRenderQueueFamilyIndex() const noexcept
	{
		return queue_family_index;
	}

	const PerFrameResources & GraphicsWorker::GetPerFrameResources() const noexcept
	{
		return per_frame_resources;
	}

	hrs::expected<Swapchain, GraphicsWorker::SwapchainRecreateError>
	GraphicsWorker::create_swapchain(Device *_device,
									 const Surface *_surface,
									 vk::Extent2D resolution,
									 vk::PresentModeKHR swapchain_present_mode,
									 vk::SwapchainKHR old_swapchain)
	{
		vk::PhysicalDevice ph_device = _device->GetPhysicalDevice();

		auto [surface_caps_res, surface_caps] = ph_device.getSurfaceCapabilitiesKHR(_surface->GetSurface());
		if(surface_caps_res != vk::Result::eSuccess)
			return {surface_caps_res};

		bool is_must_be_null = false;
		vk::Extent2D choosed_extent = _surface->ClampExtent(surface_caps, resolution);
		if(choosed_extent.height == 0 && choosed_extent.width == 0)
			is_must_be_null = true;

		if(is_must_be_null)
			return Swapchain::CreateNull(_device);

		auto [surface_formats_res, surface_formats] = ph_device.getSurfaceFormatsKHR(_surface->GetSurface());
		if(surface_formats_res != vk::Result::eSuccess)
			return {surface_formats_res};

		vk::SurfaceFormatKHR choosed_swapchain_format = surface_formats[0];

		constexpr static vk::ImageUsageFlags desired_swapchain_usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eTransferDst;

		if(!_surface->IsImageUsageSupported(surface_caps, desired_swapchain_usage))
			return {GraphicsWorkerResult::UnsupportedSwapchainImageUsage};

		constexpr static vk::CompositeAlphaFlagBitsKHR composite_alpha_priorities[] =
		{
			vk::CompositeAlphaFlagBitsKHR::eInherit,
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
			vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
		};

		vk::CompositeAlphaFlagBitsKHR composite_alpha;
		for(const auto alpha : composite_alpha_priorities)
			if(_surface->IsCompositeAlphaSupported(surface_caps, alpha))
			{
				composite_alpha = alpha;
				break;
			}

		vk::SwapchainCreateInfoKHR info({},
										_surface->GetSurface(),
										_surface->ClampImageCount(surface_caps, MIN_SWAPCHAIN_IMAGES_COUNT),
										choosed_swapchain_format.format,
										choosed_swapchain_format.colorSpace,
										choosed_extent,
										1,
										desired_swapchain_usage,
										vk::SharingMode::eExclusive,
										{},
										vk::SurfaceTransformFlagBitsKHR::eIdentity,
										composite_alpha,
										swapchain_present_mode,
										VK_TRUE,
										old_swapchain);

#warning RVO violation????????????????????
		auto swapchain_exp = Swapchain::Create(_device, info);
		if(swapchain_exp)
			return std::move(swapchain_exp.value());
		else
			return {swapchain_exp.error()};
	}

	hrs::expected<ImageFormatBounds, AllocationError>
	GraphicsWorker::create_renderpasses_output_image(Device *_device,
													 vk::Format swapchain_format,
													 const vk::Extent2D &resolution)
	{

	}

	GraphicsWorker::SwapchainAcquireResult GraphicsWorker::acquire_swapchain_image(const RenderInputs &inputs)
	{
		vk::Device device_handle = device->GetDevice();
		bool is_fences_waited = false;
		if(!swapchain.IsCreated())
		{
			auto swapchain_exp = create_swapchain(device,
												  surface,
												  inputs.resolution,
												  inputs.present_mode,
												  VK_NULL_HANDLE);

			if(!swapchain_exp)
				return SwapchainAcquireResult(swapchain_exp.error());

			auto renderpasses_output_image_exp =
				create_renderpasses_output_image(device,
												 swapchain_exp.value().GetSurfaceFormat().format,
												 inputs.resolution);

			if(!renderpasses_output_image_exp)
				return SwapchainAcquireResult(renderpasses_output_image_exp.error());

			swapchain = std::move(swapchain_exp.value());
			renderpasses_output_image = std::move(renderpasses_output_image_exp.value());
			is_swapchain_needs_to_be_recreated = false;

			if(!swapchain.IsCreated())
				return SwapchainAcquireResult(vk::Result::eErrorOutOfDateKHR);//vk::Result::eSuccess;
		}

		if(is_swapchain_needs_to_be_recreated || inputs.is_resized)
		{
			/*auto wait_res = device_handle.waitForFences(per_frame_resources.fences,
														VK_TRUE,
														std::numeric_limits<std::uint64_t>::max());

			if(wait_res != vk::Result::eSuccess)
				return SwapchainAcquireResult(wait_res);
			*/

			auto queue_wait_res = render_queue.waitIdle();
			if(queue_wait_res != vk::Result::eSuccess)
				return SwapchainAcquireResult(queue_wait_res);

			is_fences_waited = true;

			is_swapchain_needs_to_be_recreated = false;
			vk::SwapchainKHR retired_swapchain = swapchain.RetireAndDestroy();
			auto swapchain_exp = create_swapchain(device,
												  surface,
												  inputs.resolution,
												  inputs.present_mode,
												  retired_swapchain);

			device_handle.destroy(retired_swapchain);
			device->GetAllocator()->Destroy(renderpasses_output_image.image, false);

			if(!swapchain_exp)
			{
				swapchain = Swapchain::CreateNull(device);
				return SwapchainAcquireResult(swapchain_exp.error());
			}

			auto renderpasses_output_image_exp =
				create_renderpasses_output_image(device,
												 swapchain_exp.value().GetSurfaceFormat().format,
												 inputs.resolution);

			if(!renderpasses_output_image_exp)
				return SwapchainAcquireResult(renderpasses_output_image_exp.error());

			swapchain = std::move(swapchain_exp.value());
			renderpasses_output_image = std::move(renderpasses_output_image_exp.value());

			if(!swapchain.IsCreated())
				return SwapchainAcquireResult(vk::Result::eErrorOutOfDateKHR);//vk::Result::eSuccess;
		}

		auto [acquire_res, acquire_image_index] = swapchain.AcquireNextImage(VK_NULL_HANDLE);
		switch(acquire_res)
		{
			case vk::Result::eSuccess:
				break;
			case vk::Result::eSuboptimalKHR:
				break;
			case vk::Result::eErrorOutOfDateKHR:
				is_swapchain_needs_to_be_recreated = true;
			default:
				return SwapchainAcquireResult(acquire_res);
				break;
		}

		return SwapchainAcquireResult(acquire_res, acquire_image_index, is_fences_waited);
	}

	vk::Result GraphicsWorker::present_swapchain_image(std::uint32_t image_index) noexcept
	{
		const vk::Semaphore wait_semaphores[] = {swapchain.GetPresentWaitSemaphore()};
		const std::uint32_t image_indices[] = {image_index};
		auto present_res = swapchain.Present(render_queue,
											 wait_semaphores,
											 image_indices);

		switch(present_res)
		{
			case vk::Result::eErrorOutOfDateKHR:
				is_swapchain_needs_to_be_recreated = true;
				break;
			default:
				break;
		}


		return present_res;
	}
};
