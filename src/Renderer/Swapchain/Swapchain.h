#pragma once

#include <cstdint>
#include <numeric>
#include "hrs/non_creatable.hpp"
#include "hrs/expected.hpp"
#include "hrs/error.hpp"
#include "../Vulkan/VulkanInclude.hpp"

namespace FireLand
{
	class Device;
	class Surface;

	struct SwapchainRecreateInfo
	{
		const Surface *surface;
		vk::SwapchainCreateFlagsKHR flags;
		std::uint32_t min_image_count;
		vk::SurfaceFormatKHR surface_format;
		vk::Extent2D resolution;
		std::uint32_t image_array_layers;
		vk::ImageUsageFlags image_usage;
		vk::SurfaceTransformFlagBitsKHR pre_transform;
		vk::CompositeAlphaFlagBitsKHR composite_alpha;
		vk::PresentModeKHR present_mode;
		vk::Bool32 clipped;
		vk::Queue present_queue;
	};

	class RetiredSwapchain : public hrs::non_copyable
	{
	public:
		RetiredSwapchain(Device *_parent_device, vk::SwapchainKHR _swapchain) noexcept;
		~RetiredSwapchain();
		RetiredSwapchain(RetiredSwapchain &&rsc) noexcept;
		RetiredSwapchain & operator=(RetiredSwapchain &&rsc) noexcept;

		void Destroy() noexcept;

		explicit operator bool() const noexcept;
		bool IsCreated() const noexcept;

		Device * GetParentDevice() noexcept;
		const Device * GetParentDevice() const noexcept;
		vk::SwapchainKHR GetHandle() const noexcept;

		vk::SwapchainKHR Release() noexcept;

	private:
		Device *parent_device;
		vk::SwapchainKHR swapchain;
	};

	class Swapchain : public hrs::non_copyable
	{
	public:
		Swapchain(Device *_parent_device) noexcept;
		~Swapchain();
		Swapchain(Swapchain &&sc) noexcept;
		Swapchain & operator=(Swapchain &&sc) noexcept;

		vk::Result Recreate(const SwapchainRecreateInfo &info,
							const RetiredSwapchain &retired) noexcept;

		void Destroy() noexcept;

		explicit operator bool() const noexcept;
		bool IsCreated() const noexcept;
		bool IsRetired() const noexcept;

		hrs::expected<std::pair<::std::uint32_t, vk::Semaphore>, vk::Result>
		AcquireImage(vk::Fence fence,
					 std::uint64_t timeout = std::numeric_limits<std::uint64_t>::max()) noexcept;

		vk::Result Present(std::span<const vk::Semaphore> wait_semaphores,
						   std::span<const std::uint32_t> image_indices) noexcept;

		RetiredSwapchain Retire() noexcept;

		Device * GetParentDevice() noexcept;
		const Device * GetParentDevice() const noexcept;
		const Surface * GetSurface() const noexcept;
		vk::SwapchainKHR GetHandle() const noexcept;
		const std::vector<vk::Image> & GetImages() const noexcept;
		const vk::Extent2D & GetResolution() const noexcept;
		vk::PresentModeKHR GetPresentMode() const noexcept;
		const vk::SurfaceFormatKHR & GetSurfaceFormat() const noexcept;
		vk::Queue GetPresentQueue() const noexcept;

	private:
		Device *parent_device;
		const Surface *surface;
		vk::SwapchainKHR swapchain;
		std::vector<vk::Image> images;
		vk::Extent2D resolution;
		vk::PresentModeKHR present_mode;
		vk::SurfaceFormatKHR surface_format;
		vk::Queue present_queue;
		std::vector<vk::Semaphore> acquire_signal_semaphores;
		std::uint32_t acquire_index;
	};
};
