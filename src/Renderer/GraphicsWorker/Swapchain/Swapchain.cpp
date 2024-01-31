#include "Swapchain.h"
#include "../../../Vulkan/VulkanUtils.hpp"
#include "../../../hrs/scoped_call.hpp"

namespace FireLand
{
	void Swapchain::init(vk::SwapchainKHR _swapchain,
						 std::vector<vk::Image> &&_images,
						 vk::SurfaceFormatKHR _format,
						 vk::PresentModeKHR _present_mode,
						 vk::Extent2D _extent,
						 vk::Semaphore _acquire_signal_semaphore,
						vk::Semaphore _present_wait_semaphore) noexcept
	{
		swapchain = _swapchain;
		images = std::move(_images);
		format = _format;
		present_mode = _present_mode;
		extent = _extent;
		acquire_signal_semaphore = _acquire_signal_semaphore;
		present_wait_semaphore = _present_wait_semaphore;
	}

	Swapchain::Swapchain(const Device *_parent_device) noexcept
		: parent_device(_parent_device)
	{
		hrs::assert_true_debug(parent_device, "Parent device pointer points to null!");
		hrs::assert_true_debug(parent_device->GetDevice(), "Parent device isn't createdd yet!");
	}

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

	vk::Result
	Swapchain::Recreate(const vk::SwapchainCreateInfoKHR &info,
						vk::Semaphore _acquire_signal_semaphore,
						vk::Semaphore _present_wait_semaphore)
	{
		Destroy();
		if(IsBadExtent(info.imageExtent))
			return vk::Result::eSuccess;

		vk::Device device_handle = parent_device->GetDevice();
		constexpr static vk::SemaphoreCreateInfo sem_info;
		hrs::scoped_call semaphores_dtor([&]()
		{
			device_handle.destroy(_acquire_signal_semaphore);
			device_handle.destroy(_present_wait_semaphore);
		});

		if(!_acquire_signal_semaphore)
		{
			auto [_acquire_sem_res, _acquire_sem] = device_handle.createSemaphore(sem_info);
			if(_acquire_sem_res != vk::Result::eSuccess)
				return _acquire_sem_res;

			_acquire_signal_semaphore = _acquire_sem;
		}

		if(!_present_wait_semaphore)
		{
			auto [_present_sem_res, _present_sem] = device_handle.createSemaphore(sem_info);
			if(_present_sem_res != vk::Result::eSuccess)
				return _present_sem_res;

			_present_wait_semaphore = _present_sem;
		}

		auto [u_swapchain_res, u_swapchain] = device_handle.createSwapchainKHRUnique(info);
		if(u_swapchain_res != vk::Result::eSuccess)
			return u_swapchain_res;

		auto [_images_res, _images] = device_handle.getSwapchainImagesKHR(u_swapchain.get());
		if(_images_res != vk::Result::eSuccess)
			return _images_res;

		semaphores_dtor.Drop();

		init(u_swapchain.release(),
			 std::move(_images),
			 {info.imageFormat, info.imageColorSpace},
			 info.presentMode,
			 info.imageExtent,
			 _acquire_signal_semaphore,
			 _present_wait_semaphore);

		return vk::Result::eSuccess;
	}

	std::tuple<vk::SwapchainKHR, vk::Semaphore, vk::Semaphore> Swapchain::RetireAndDestroy() noexcept
	{
		std::tuple out_tpl(swapchain, acquire_signal_semaphore, present_wait_semaphore);
		if(IsCreated())
		{
			images.clear();
			swapchain = VK_NULL_HANDLE;
			acquire_signal_semaphore = VK_NULL_HANDLE;
			present_wait_semaphore = VK_NULL_HANDLE;
		}

		return out_tpl;
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
