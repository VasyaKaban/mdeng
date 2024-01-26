#include "Swapchain.h"
#include "../../../Vulkan/VulkanUtils.hpp"

namespace FireLand
{
	Swapchain::Swapchain(const Device *_parent_device,
						 vk::SwapchainKHR _swapchain,
						 std::vector<vk::Image> &&_images,
						 vk::SurfaceFormatKHR _format,
						 vk::PresentModeKHR _present_mode,
						 vk::Extent2D _extent,
						 vk::Semaphore _acquire_signal_semaphore,
						 vk::Semaphore _present_wait_semaphore) noexcept
		: parent_device(_parent_device),
		  swapchain(_swapchain),
		  images(std::move(_images)),
		  format(_format),
		  present_mode(_present_mode),
		  extent(_extent),
		  acquire_signal_semaphore(_acquire_signal_semaphore),
		  present_wait_semaphore(_present_wait_semaphore) {}

	Swapchain::~Swapchain()
	{
		Destroy();
	}

	Swapchain::Swapchain(Swapchain &&schain) noexcept
		: parent_device(schain.parent_device),
		  swapchain(schain.swapchain),
		  images(std::move(schain.images)),
		  format(schain.format),
		  present_mode(schain.present_mode),
		  extent(schain.extent),
		  acquire_signal_semaphore(schain.acquire_signal_semaphore),
		  present_wait_semaphore(schain.present_wait_semaphore)
	{
		schain.acquire_signal_semaphore = VK_NULL_HANDLE;
		schain.present_wait_semaphore = VK_NULL_HANDLE;
		schain.swapchain = VK_NULL_HANDLE;
	}

	Swapchain & Swapchain::operator=(Swapchain &&schain) noexcept
	{
		Destroy();

		parent_device = schain.parent_device;
		swapchain = schain.swapchain;
		images = std::move(schain.images);
		format = schain.format;
		present_mode = schain.present_mode;
		extent = schain.extent;
		acquire_signal_semaphore = schain.acquire_signal_semaphore;
		present_wait_semaphore = schain.present_wait_semaphore;

		schain.acquire_signal_semaphore = VK_NULL_HANDLE;
		schain.present_wait_semaphore = VK_NULL_HANDLE;
		schain.swapchain = VK_NULL_HANDLE;

		return *this;
	}

	hrs::expected<Swapchain, vk::Result>
	Swapchain::Create(const Device *_parent_device,
					  const vk::SwapchainCreateInfoKHR &info)
	{
		hrs::assert_true_debug(_parent_device, "Parent device points to null!");
		hrs::assert_true_debug(_parent_device->GetDevice(), "Device isn't created yet!");

		if(IsBadExtent(info.imageExtent))
			return Swapchain(_parent_device,
							 {},
							 {},
							 {info.imageFormat, info.imageColorSpace},
							 info.presentMode,
							 info.imageExtent,
							 {},
							 {});


		vk::Device device_handle = _parent_device->GetDevice();
		auto [u_swapchain_res, u_swapchain] = device_handle.createSwapchainKHRUnique(info);
		if(u_swapchain_res != vk::Result::eSuccess)
			return u_swapchain_res;

		const static vk::SemaphoreCreateInfo sem_info;
		auto [u_acquire_sem_res, u_acquire_sem] = device_handle.createSemaphoreUnique(sem_info);
		if(u_acquire_sem_res != vk::Result::eSuccess)
			return u_acquire_sem_res;

		auto [u_present_sem_res, u_present_sem] = device_handle.createSemaphoreUnique(sem_info);
		if(u_present_sem_res != vk::Result::eSuccess)
			return u_present_sem_res;

		auto [_images_res, _images] = device_handle.getSwapchainImagesKHR(u_swapchain.get());
		if(_images_res != vk::Result::eSuccess)
			return _images_res;

		return Swapchain(_parent_device,
						 u_swapchain.release(),
						 std::move(_images),
						 {info.imageFormat, info.imageColorSpace},
						 info.presentMode,
						 info.imageExtent,
						 u_acquire_sem.release(),
						 u_present_sem.release());
	}

	vk::SwapchainKHR Swapchain::RetireAndDestroy() noexcept
	{
		const vk::SwapchainKHR out_swapchain = swapchain;
		if(IsCreated())
		{
			vk::Device device_handle = parent_device->GetDevice();
			device_handle.destroy(acquire_signal_semaphore);
			device_handle.destroy(present_wait_semaphore);
			images.clear();
			swapchain = VK_NULL_HANDLE;
			acquire_signal_semaphore = VK_NULL_HANDLE;
			present_wait_semaphore = VK_NULL_HANDLE;
		}

		return out_swapchain;
	}

	void Swapchain::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		vk::Device device_handle = parent_device->GetDevice();
		device_handle.destroy(swapchain);
		device_handle.destroy(acquire_signal_semaphore);
		device_handle.destroy(present_wait_semaphore);
		images.clear();
		swapchain = VK_NULL_HANDLE;
		acquire_signal_semaphore = VK_NULL_HANDLE;
		present_wait_semaphore = VK_NULL_HANDLE;
	}

	bool Swapchain::IsCreated() const noexcept
	{
		return swapchain;
	}

	const Device * Swapchain::GetParentDevice() noexcept
	{
		return parent_device;
	}

	const std::vector<vk::Image> & Swapchain::GetImages() const noexcept
	{
		return images;
	}

	vk::Image Swapchain::GetImage(std::size_t index) const noexcept
	{
		return images[index];
	}

	vk::SurfaceFormatKHR Swapchain::GetSurfaceFormat() const noexcept
	{
		return format;
	}

	vk::PresentModeKHR Swapchain::GetPresentMode() const noexcept
	{
		return present_mode;
	}

	vk::SwapchainKHR Swapchain::GetHandle() const noexcept
	{
		return swapchain;
	}

	vk::Extent2D Swapchain::GetExtent() const noexcept
	{
		return extent;
	}

	vk::Semaphore Swapchain::GetAcquireSignalSemaphore() const noexcept
	{
		return acquire_signal_semaphore;
	}

	vk::Semaphore Swapchain::GetPresentWaitSemaphore() const noexcept
	{
		return present_wait_semaphore;
	}

	vk::ResultValue<std::uint32_t>
	Swapchain::AcquireNextImage(vk::Fence fence,
								std::uint64_t acquire_timeout) noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Swapchain isn't created yet!");

		return parent_device->GetDevice().acquireNextImageKHR(swapchain,
															  acquire_timeout,
															  acquire_signal_semaphore,
															  fence);
	}

	vk::Result Swapchain::Present(vk::Queue queue,
								  std::span<const vk::Semaphore> wait_semaphores,
								  std::span<const std::uint32_t> image_indices)
	{
		hrs::assert_true_debug(IsCreated(), "Swapchain isn't created yet!");
		const vk::SwapchainKHR swapchains[] = {swapchain};
		const vk::PresentInfoKHR info(wait_semaphores,
									  swapchains,
									  image_indices,
									  {});

		return queue.presentKHR(info);
	}
};
