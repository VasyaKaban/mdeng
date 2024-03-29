#include "GraphicsWorker.h"
#include "../../Vulkan/UnexpectedVkResult.hpp"
#include "../../hrs/scoped_call.hpp"

namespace FireLand
{
	GraphicsWorker::GraphicsWorker(Device *_device,
								   FlaggedSwapchain &&_flagged_swapchain,
								   RenderpassesOutputImage &&_renderpasses_output_image,
								   vk::Queue _render_queue,
								   std::uint32_t _queue_family_index,
								   PerFrameResources &&_per_frame_resources,
								   std::tuple<DefferedPass> &&_renderpasses)
		: device(_device),
		  flagged_swapchain(std::move(_flagged_swapchain)),
		  renderpasses_output_image(std::move(_renderpasses_output_image)),
		  render_queue(_render_queue),
		  queue_family_index(_queue_family_index),
		  per_frame_resources(std::move(_per_frame_resources)),
		  renderpasses(std::move(_renderpasses)){}

	GraphicsWorker::~GraphicsWorker()
	{
		Destroy();
	}

	void GraphicsWorker::Destroy()
	{
		/*!!!!!! [[maybe_unused]]auto _ = render_queue.waitIdle();*/
		destroy_renderpasses();
		flagged_swapchain.Destroy();
		vk::Device device_handle = device->GetDevice();
		renderpasses_output_image.Destroy();
		per_frame_resources.Destroy();
	}

	GraphicsWorker::GraphicsWorker(GraphicsWorker &&worker) noexcept
		: device(worker.device),
		  flagged_swapchain(std::move(worker.flagged_swapchain)),
		  renderpasses_output_image(std::move(worker.renderpasses_output_image)),
		  render_queue(worker.render_queue),
		  queue_family_index(worker.queue_family_index),
		  per_frame_resources(std::move(worker.per_frame_resources)),
		  renderpasses(std::move(worker.renderpasses)) {}

	hrs::expected<GraphicsWorker, hrs::unexpected_result>
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
		vk::Queue _render_queue = device_handle.getQueue(_queue_family_index, queue_index);

		FlaggedSwapchain _flagged_swapchain(_device);
		auto swapchain_unexpected_res = _flagged_swapchain.Recreate(_surface,
																	resolution,
																	swapchain_present_mode);

		if(swapchain_unexpected_res)
			return swapchain_unexpected_res;

		RenderpassesOutputImage _renderpasses_out_img(_device->GetAllocator());
		auto renderpasses_out_img_unexpected_res =
			_renderpasses_out_img.Recreate(_device->GetPhysicalDevice(),
										   _flagged_swapchain.GetSwapchain().GetSurfaceFormat().format,
										   SUBPASSES_OUTPUT_IMAGE_FORMAT_FEATURES,
										   SUBPASSES_OUTPUT_IMAGE_IMAGE_USAGE,
										   _flagged_swapchain.GetSwapchain().GetExtent());

		if(renderpasses_out_img_unexpected_res)
			return renderpasses_out_img_unexpected_res;

		auto renderpasses_exp = create_renderpasses(_device,
													_renderpasses_out_img,
													_flagged_swapchain.GetSwapchain().GetExtent(),
													frames_count);

		if(!renderpasses_exp)
			return std::move(renderpasses_exp.error());

		PerFrameResources _per_frame_resources(_device->GetDevice());
		auto per_frame_resources_unexpected_res = _per_frame_resources.Recreate(_queue_family_index,
																				frames_count);

		if(per_frame_resources_unexpected_res != vk::Result::eSuccess)
			return {UnexpectedVkResult(per_frame_resources_unexpected_res)};

		return GraphicsWorker(_device,
							  std::move(_flagged_swapchain),
							  std::move(_renderpasses_out_img),
							  _render_queue,
							  _queue_family_index,
							  std::move(_per_frame_resources),
							  std::move(renderpasses_exp.value()));
	}

	hrs::unexpected_result GraphicsWorker::Render(const RenderInputs &inputs)
	{
		/*
		 * Success, timeout(no), not ready(no), suboptimal -> success op -> semaphore is signaled
		 * Error -> error op -> semaphore isn't signaled
		 */

		auto [acquire_unexpected_res, acquire_res, acquire_index, is_fences_waited] =
			acquire_swapchain_image(inputs);
		if(acquire_unexpected_res)
			return std::move(acquire_unexpected_res);

		switch(acquire_res)
		{
			case vk::Result::eSuccess:
			case vk::Result::eSuboptimalKHR:
				break;
			default:
				hrs::assert_true(false, "Bad result!");
				break;
		}

		auto render_result = render(inputs, is_fences_waited, acquire_index);
		if(render_result != vk::Result::eSuccess)
			return UnexpectedVkResult(render_result);

		per_frame_resources.UpdateTargetFrameIndex();
		auto present_res = present_swapchain_image(acquire_index);
		switch(present_res)
		{
			case vk::Result::eSuccess:
			case vk::Result::eSuboptimalKHR:
			case vk::Result::eErrorOutOfDateKHR:
				break;
			default:
				return UnexpectedVkResult(present_res);
				break;
		}

		return {};
	}

	Device * GraphicsWorker::GetDevice() noexcept
	{
		return device;
	}

	const Device * GraphicsWorker::GetDevice() const noexcept
	{
		return device;
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

	void GraphicsWorker::destroy_renderpasses() noexcept
	{
		std::apply([](auto &...rpasses)
		{
			(rpasses.Destroy(), ...);
		}, renderpasses);
	}

	void GraphicsWorker::destroy_renderpasses_without_shaders() noexcept
	{
		std::apply([](auto &...rpasses)
		{
			(rpasses.DestroyWithoutShaders(), ...);
		}, renderpasses);
	}

	hrs::unexpected_result GraphicsWorker::recreate_and_notify(const RenderInputs &inputs)
	{
		hrs::scoped_call cleanup([this]()
		{
			destroy_renderpasses_without_shaders();
			renderpasses_output_image.Destroy();
			flagged_swapchain.Destroy();
		});

		auto swapchain_unexpected_res = flagged_swapchain.Recreate(flagged_swapchain.GetSurface(),
																   inputs.resolution,
																   inputs.present_mode);

		if(swapchain_unexpected_res)
			return swapchain_unexpected_res;

		auto rpasses_out_img_unexpected_res =
			renderpasses_output_image.Recreate(device->GetPhysicalDevice(),
											   flagged_swapchain.GetSwapchain().GetSurfaceFormat().format,
											   SUBPASSES_OUTPUT_IMAGE_FORMAT_FEATURES,
											   SUBPASSES_OUTPUT_IMAGE_IMAGE_USAGE,
											   flagged_swapchain.GetSwapchain().GetExtent());

		if(rpasses_out_img_unexpected_res)
			return rpasses_out_img_unexpected_res;

		///NOTIFY RESIZE
		auto unexpected_notify_res = notify_renderpasses_resize(renderpasses_output_image,
																flagged_swapchain);
		if(unexpected_notify_res)
			return unexpected_notify_res;

		cleanup.Drop();
		return {};
	}

	GraphicsWorker::SwapchainAcquireResult GraphicsWorker::acquire_swapchain_image(const RenderInputs &inputs)
	{
		bool is_fences_waited = false;
		if(!flagged_swapchain.IsCreated())
		{
			auto unexpected_recreate_res = recreate_and_notify(inputs);
			if(unexpected_recreate_res)
				return SwapchainAcquireResult(std::move(unexpected_recreate_res));
		}
		else if(flagged_swapchain.IsRecreationNeeded() || inputs.is_resized)
		{
			auto queue_wait_res = render_queue.waitIdle();//instead of fences wait!
			if(queue_wait_res != vk::Result::eSuccess)
				return SwapchainAcquireResult(UnexpectedVkResult(queue_wait_res));

			is_fences_waited = true;

			auto unexpected_recreate_res = recreate_and_notify(inputs);
			if(unexpected_recreate_res)
				return SwapchainAcquireResult(std::move(unexpected_recreate_res));
		}

		if(!flagged_swapchain.IsCreated())
			return SwapchainAcquireResult({}, vk::Result::eErrorOutOfDateKHR);//vk::Result::eSuccess

		auto [acquire_res, acquire_image_index] = flagged_swapchain.GetSwapchain().AcquireNextImage(VK_NULL_HANDLE);
		switch(acquire_res)
		{
			case vk::Result::eSuccess:
				break;
			case vk::Result::eSuboptimalKHR:
				break;
			case vk::Result::eErrorOutOfDateKHR:
				flagged_swapchain.SetRecreationNeed();
			default:
				return SwapchainAcquireResult(UnexpectedVkResult(acquire_res));
				break;
		}

		return SwapchainAcquireResult({}, acquire_res, acquire_image_index, is_fences_waited);
	}

	vk::Result GraphicsWorker::present_swapchain_image(std::uint32_t image_index) noexcept
	{
		Swapchain &swapchain = flagged_swapchain.GetSwapchain();
		const vk::Semaphore wait_semaphores[] = {swapchain.GetPresentWaitSemaphore()};
		const std::uint32_t image_indices[] = {image_index};
		auto present_res = swapchain.Present(render_queue,
											 wait_semaphores,
											 image_indices);

		switch(present_res)
		{
			case vk::Result::eErrorOutOfDateKHR:
				flagged_swapchain.SetRecreationNeed();
				break;
			default:
				break;
		}


		return present_res;
	}

	vk::Result GraphicsWorker::render(const RenderInputs &inputs,
									  bool is_fences_waited,
									  std::uint32_t acquire_index) noexcept
	{
		Swapchain &swapchain = flagged_swapchain.GetSwapchain();
		vk::Device device_handle = device->GetDevice();
		vk::CommandBuffer comm_buffer = per_frame_resources.GetTargetCommandBuffer();
		vk::Fence fence = per_frame_resources.GetTargetFence();
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

		std::apply([comm_buffer, &inputs, this](auto &...render_passes)
		{
			(render_passes.Render(comm_buffer,
								  per_frame_resources.GetTargetFrameIndex(),
								  VK_NULL_HANDLE/*globals_set*/,
								  inputs.view_point,
								  inputs.scene), ...);
		}, renderpasses);

		//render

		/*
		 renderpasses_output_image -> transfer_src_optimal
		 acquired_image -> transfer_dst_optimal
		 image_copy
		 acquired_image -> present_src_optimal
		 */

		const vk::ImageSubresourceRange
			layout_change_subres_range(vk::ImageAspectFlagBits::eColor,
									   0, 1, 0, 1);

		const vk::ImageMemoryBarrier
			output_image_to_optimal_src_barrier(vk::AccessFlagBits::eColorAttachmentWrite,
												vk::AccessFlagBits::eTransferRead,
												vk::ImageLayout::eColorAttachmentOptimal,
												vk::ImageLayout::eTransferSrcOptimal,
												VK_QUEUE_FAMILY_IGNORED,
												VK_QUEUE_FAMILY_IGNORED,
												renderpasses_output_image.GetBoundedImage().image,
												layout_change_subres_range);

		const vk::ImageMemoryBarrier
			acquire_image_to_optimal_dst_barrier({},
												 {},
												 vk::ImageLayout::eUndefined,
												 vk::ImageLayout::eTransferDstOptimal,
												 VK_QUEUE_FAMILY_IGNORED,
												 VK_QUEUE_FAMILY_IGNORED,
												 acquired_image,
												 layout_change_subres_range);

		const vk::ImageMemoryBarrier
			acquire_image_to_present_src_barrier({},
												 {},
												 vk::ImageLayout::eTransferDstOptimal,
												 vk::ImageLayout::ePresentSrcKHR,
												 VK_QUEUE_FAMILY_IGNORED,
												 VK_QUEUE_FAMILY_IGNORED,
												 acquired_image,
												 layout_change_subres_range);

		comm_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
									vk::PipelineStageFlagBits::eTransfer,
									vk::DependencyFlagBits::eByRegion,
									{},
									{},
									output_image_to_optimal_src_barrier);

		comm_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eBottomOfPipe,
									vk::PipelineStageFlagBits::eTransfer,
									vk::DependencyFlagBits::eByRegion,
									{},
									{},
									acquire_image_to_optimal_dst_barrier);

		const vk::Extent2D &resolution = swapchain.GetExtent();

		const vk::ImageSubresourceLayers image_layers(vk::ImageAspectFlagBits::eColor,
													  0,
													  0,
													  1);

		const vk::ImageBlit blit_region(image_layers,
										{vk::Offset3D(0, 0, 0), vk::Offset3D(resolution.width, resolution.height, 1)},
										image_layers,
										{vk::Offset3D(0, 0, 0), vk::Offset3D(resolution.width, resolution.height, 1)});

		comm_buffer.blitImage(renderpasses_output_image.GetBoundedImage().image,
							  vk::ImageLayout::eTransferSrcOptimal,
							  acquired_image,
							  vk::ImageLayout::eTransferDstOptimal,
							  blit_region,
							  vk::Filter::eNearest);

		comm_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
									vk::PipelineStageFlagBits::eTopOfPipe,
									vk::DependencyFlagBits::eByRegion,
									{},
									{},
									acquire_image_to_present_src_barrier);


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

		return render_queue.submit(submit_info, fence);
	}

	hrs::expected<std::tuple<DefferedPass>, hrs::unexpected_result>
	GraphicsWorker::create_renderpasses(Device *_device,
										const RenderpassesOutputImage &out_image,
										const vk::Extent2D &resolution,
										std::uint32_t count)
	{
		DefferedPass deffered_pass(_device);

		auto deffered_unexpected_res  = deffered_pass.Resize(resolution,
															out_image.GetBoundedImage().image,
															out_image.GetFormat(),
															VK_NULL_HANDLE,
															count);

		if(deffered_unexpected_res)
			return deffered_unexpected_res;

		return std::tuple(std::move(deffered_pass));
	}

	hrs::unexpected_result GraphicsWorker::notify_renderpasses_resize(const RenderpassesOutputImage &_out_image,
																	  const FlaggedSwapchain &_flagged_swapchain)
	{
		auto &deffered_pass = std::get<RenderPassIndices::DefferedPassIndex>(renderpasses);
		auto resize_unexpected_res = deffered_pass.Resize(_flagged_swapchain.GetSwapchain().GetExtent(),
														  _out_image.GetBoundedImage().image,
														  _out_image.GetFormat(),
														  VK_NULL_HANDLE,/*globals set*/
														  per_frame_resources.GetCount());

		if(resize_unexpected_res)
			return resize_unexpected_res;

		return {};
	}
};
