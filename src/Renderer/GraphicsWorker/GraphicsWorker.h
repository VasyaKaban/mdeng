#pragma once

#include "../Context/DeviceWorker.h"
#include "../Context/Device.h"
#include "../Context/Surface.h"
#include "FlaggedSwapchain.h"
#include "../../hrs/expected.hpp"
#include "../../hrs/error.hpp"
#include "PerFrameResources.h"
#include "RenderInputs.hpp"
#include "RenderpassesOutputImage.h"
#include "RenderPass/DefferedPass/DefferedPass.h"

namespace FireLand
{
	class GraphicsWorker : public DeviceWorker, public::hrs::non_copyable, public hrs::non_move_assignable
	{
		constexpr static vk::FormatFeatureFlags SUBPASSES_OUTPUT_IMAGE_FORMAT_FEATURES =
			vk::FormatFeatureFlagBits::eColorAttachmentBlend;

		constexpr static vk::ImageUsageFlags SUBPASSES_OUTPUT_IMAGE_IMAGE_USAGE =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eInputAttachment |
			vk::ImageUsageFlagBits::eTransferSrc;

		GraphicsWorker(Device *_device,
					   FlaggedSwapchain &&_flagged_swapchain,
					   RenderpassesOutputImage &&_renderpasses_output_image,
					   vk::Queue _render_queue,
					   std::uint32_t _queue_family_index,
					   PerFrameResources &&_per_frame_resources,
					   std::tuple<DefferedPass> &&_renderpasses);
	public:

		enum RenderPassIndices
		{
			DefferedPassIndex = 0,
			LastUnusedRenderPassIndex
		};

		virtual ~GraphicsWorker() override;
		virtual void Destroy() override;

		GraphicsWorker(GraphicsWorker &&worker) noexcept;

		static hrs::expected<GraphicsWorker, hrs::unexpected_result>
		Create(Device *_device,
			   const Surface *_surface,
			   std::uint32_t _queue_family_index,
			   std::uint32_t queue_index,
			   vk::Extent2D resolution,
			   vk::PresentModeKHR swapchain_present_mode,
			   std::uint32_t frames_count = 2);

		hrs::unexpected_result Render(const RenderInputs &inputs);

		Device * GetDevice() noexcept;
		const Device * GetDevice() const noexcept;
		const Swapchain & GetSwapchain() const noexcept;
		vk::Queue GetRenderQueue() const noexcept;
		std::uint32_t GetRenderQueueFamilyIndex() const noexcept;
		const PerFrameResources & GetPerFrameResources() const noexcept;

		DefferedPass & GetDefferedPass() noexcept
		{
			return std::get<0>(renderpasses);
		}

	private:

		struct SwapchainAcquireResult
		{
			hrs::unexpected_result unexpected_result;
			vk::Result vk_result;
			std::uint32_t image_index;
			bool is_fences_waited;
		};

		void destroy_renderpasses() noexcept;
		void destroy_renderpasses_without_shaders() noexcept;

		hrs::unexpected_result recreate_and_notify(const RenderInputs &inputs);

		SwapchainAcquireResult acquire_swapchain_image(const RenderInputs &inputs);
		vk::Result present_swapchain_image(std::uint32_t image_index) noexcept;

		vk::Result render(const RenderInputs &inputs,
						  bool is_fences_waited,
						  std::uint32_t acquire_index) noexcept;

		static hrs::expected<std::tuple<DefferedPass>, hrs::unexpected_result>
		create_renderpasses(Device *_device,
							const RenderpassesOutputImage &out_image,
							const vk::Extent2D &resolution,
							std::uint32_t count);

		hrs::unexpected_result notify_renderpasses_resize(const RenderpassesOutputImage &_out_image,
														  const FlaggedSwapchain &_flagged_swapchain);

		Device *device;
		RenderpassesOutputImage renderpasses_output_image;
		FlaggedSwapchain flagged_swapchain;
		vk::Queue render_queue;
		std::uint32_t queue_family_index;
		PerFrameResources per_frame_resources;
		std::tuple<DefferedPass> renderpasses;
		vk::DescriptorPool descriptor_pool;
	};
};
