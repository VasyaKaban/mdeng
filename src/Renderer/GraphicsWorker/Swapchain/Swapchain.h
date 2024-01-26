#pragma once

#include "../../../hrs/expected.hpp"
#include "../../Context/Device.h"
#include "../../Context/Surface.h"
#include <span>
#include <cstdint>
#include <limits>

namespace FireLand
{
	class Swapchain : public hrs::non_copyable
	{
		Swapchain(const Device *_parent_device,
				  vk::SwapchainKHR _swapchain,
				  std::vector<vk::Image> &&_images,
				  vk::SurfaceFormatKHR _format,
				  vk::PresentModeKHR _present_mode,
				  vk::Extent2D _extent,
				  vk::Semaphore _acquire_signal_semaphore,
				  vk::Semaphore _present_wait_semaphore) noexcept;
	public:

		~Swapchain();
		Swapchain(Swapchain &&schain) noexcept;
		Swapchain & operator=(Swapchain &&schain) noexcept;

		static hrs::expected<Swapchain, vk::Result> Create(const Device *_parent_device,
														   const vk::SwapchainCreateInfoKHR &info);

		vk::SwapchainKHR RetireAndDestroy() noexcept;
		void Destroy() noexcept;

		bool IsCreated() const noexcept;

		const Device * GetParentDevice() noexcept;

		const std::vector<vk::Image> & GetImages() const noexcept;
		vk::Image GetImage(std::size_t index) const noexcept;
		vk::SurfaceFormatKHR GetSurfaceFormat() const noexcept;
		vk::PresentModeKHR GetPresentMode() const noexcept;

		vk::SwapchainKHR GetHandle() const noexcept;

		vk::Extent2D GetExtent() const noexcept;
		vk::Semaphore GetAcquireSignalSemaphore() const noexcept;
		vk::Semaphore GetPresentWaitSemaphore() const noexcept;

		vk::ResultValue<std::uint32_t>
		AcquireNextImage(vk::Fence fence,
						 std::uint64_t acquire_timeout = std::numeric_limits<std::uint64_t>::max()) noexcept;

		vk::Result Present(vk::Queue queue,
						   std::span<const vk::Semaphore> wait_semaphores,
						   std::span<const std::uint32_t> image_indices);

	private:

		const Device *parent_device;
		vk::SwapchainKHR swapchain;
		std::vector<vk::Image> images;
		vk::SurfaceFormatKHR format;
		vk::PresentModeKHR present_mode;
		vk::Extent2D extent;
		vk::Semaphore acquire_signal_semaphore;
		vk::Semaphore present_wait_semaphore;
	};
};
