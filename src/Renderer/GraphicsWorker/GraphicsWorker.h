#pragma once

#include "../Context/DeviceWorker.h"
#include "../Context/Device.h"
#include "../Context/Surface.h"
#include "Swapchain/Swapchain.h"
#include "../../hrs/expected.hpp"
#include "../../hrs/error.hpp"
#include "PerFrameResources.h"
#include "RenderInputs.hpp"
#include "RenderPass/DefferedPass/DefferedPass.h"

namespace FireLand
{
	enum class GraphicsWorkerResult
	{
		UnsupportedSwapchainImageUsage
	};

	class GraphicsWorker : public DeviceWorker, public::hrs::non_copyable, public hrs::non_move_assignable
	{
		GraphicsWorker(Device *_device,
					   const Surface *_surface,
					   Swapchain &&_swapchain,
					   ImageFormatBounds &&_renderpasses_output_image,
					   vk::Queue _render_queue,
					   std::uint32_t _queue_family_index,
					   vk::CommandPool _command_pool,
					   PerFrameResources &&_per_frame_resources);
	public:

		using SwapchainRecreateError = hrs::error<vk::Result, GraphicsWorkerResult>;
		using RecreateError = hrs::error<vk::Result, AllocationResult, GraphicsWorkerResult>;
		using RenderError = RecreateError;

		virtual ~GraphicsWorker() override;
		virtual void Destroy() override;

		constexpr static std::uint32_t MIN_SWAPCHAIN_IMAGES_COUNT = 2;

		GraphicsWorker(GraphicsWorker &&worker) noexcept;

		static hrs::expected<GraphicsWorker, RecreateError>
		Create(Device *_device,
			   const Surface *_surface,
			   std::uint32_t _queue_family_index,
			   std::uint32_t queue_index,
			   vk::Extent2D resolution,
			   vk::PresentModeKHR swapchain_present_mode,
			   std::uint32_t frames_count = 2);

		RecreateError Render(const RenderInputs &inputs);

		Device * GetDevice() noexcept;
		const Device * GetDevice() const noexcept;
		const Surface * GetSurface() const noexcept;
		const Swapchain & GetSwapchain() const noexcept;
		vk::Queue GetRenderQueue() const noexcept;
		std::uint32_t GetRenderQueueFamilyIndex() const noexcept;
		const PerFrameResources & GetPerFrameResources() const noexcept;

	private:

		static hrs::expected<Swapchain, SwapchainRecreateError>
		create_swapchain(Device *_device,
						 const Surface *_surface,
						 vk::Extent2D resolution,
						 vk::PresentModeKHR swapchain_present_mode,
						 vk::SwapchainKHR old_swapchain);

		static hrs::expected<ImageFormatBounds, AllocationError>
		create_renderpasses_output_image(Device *_device,
										 vk::Format swapchain_format,
										 const vk::Extent2D &resolution);

		struct SwapchainAcquireResult
		{
			RecreateError result;
			std::uint32_t image_index;
			bool is_fences_waited;
		};

		SwapchainAcquireResult acquire_swapchain_image(const RenderInputs &inputs);
		vk::Result present_swapchain_image(std::uint32_t image_index) noexcept;

		Device *device;
		const Surface *surface;
		ImageFormatBounds renderpasses_output_image;
		Swapchain swapchain;
		bool is_swapchain_needs_to_be_recreated;
		vk::Queue render_queue;
		std::uint32_t queue_family_index;
		vk::CommandPool command_pool;
		PerFrameResources per_frame_resources;
	};
};
