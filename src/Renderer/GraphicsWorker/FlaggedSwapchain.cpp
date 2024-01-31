#include "FlaggedSwapchain.h"
#include "../../Vulkan/UnexpectedVkResult.hpp"

namespace FireLand
{
	void FlaggedSwapchain::init(const Surface *_surface,
								Swapchain &&_swapchain,
								bool _is_recreation_needed) noexcept
	{
		surface = _surface;
		swapchain = std::move(_swapchain);
		is_recreation_needed = _is_recreation_needed;
	}

	FlaggedSwapchain::FlaggedSwapchain(const Device *_device) noexcept
		: device(_device),
		  surface(nullptr),
		  swapchain(_device) {}

	hrs::unexpected_result FlaggedSwapchain::Recreate(const Surface *_surface,
													  const vk::Extent2D &resolution,
													  vk::PresentModeKHR swapchain_present_mode)
	{	
		const Device *device = swapchain.GetParentDevice();
		if(!swapchain.IsCreated())
		{
			auto swapchain_exp = create_swapchain(device,
												  _surface,
												  resolution,
												  swapchain_present_mode);

			if(!swapchain_exp)
				return std::move(swapchain_exp.error());

			init(_surface, std::move(swapchain_exp.value()), false);
		}
		else
		{
			auto [old_swapchain, acquire_sem,  present_sem] = swapchain.RetireAndDestroy();
			auto swapchain_exp = create_swapchain(device,
												  _surface,
												  resolution,
												  swapchain_present_mode,
												  acquire_sem,
												  present_sem,
												  old_swapchain);

			device->GetDevice().destroy(old_swapchain);
			if(!swapchain_exp)
			{
				device->GetDevice().destroy(acquire_sem);
				device->GetDevice().destroy(present_sem);
				return std::move(swapchain_exp.error());
			}

			init(_surface, std::move(swapchain_exp.value()), false);
		}

		return {};
	}

	FlaggedSwapchain::~FlaggedSwapchain()
	{
		Destroy();
	}

	void FlaggedSwapchain::Destroy()
	{
		swapchain.Destroy();
		surface = nullptr;
	}

	bool FlaggedSwapchain::IsCreated() const noexcept
	{
		return swapchain.IsCreated();
	}

	Swapchain & FlaggedSwapchain::GetSwapchain() noexcept
	{
		return swapchain;
	}

	const Swapchain & FlaggedSwapchain::GetSwapchain() const noexcept
	{
		return swapchain;
	}

	const Surface * FlaggedSwapchain::GetSurface() const noexcept
	{
		return surface;
	}

	bool FlaggedSwapchain::IsRecreationNeeded() const noexcept
	{
		return is_recreation_needed;
	}

	void FlaggedSwapchain::SetRecreationNeed() noexcept
	{
		is_recreation_needed = false;
	}

	hrs::expected<Swapchain, hrs::unexpected_result>
	FlaggedSwapchain::create_swapchain(const Device *_device,
									   const Surface *_surface,
									   const vk::Extent2D &resolution,
									   vk::PresentModeKHR swapchain_present_mode,
									   vk::Semaphore _acquire_signal_semaphore,
									   vk::Semaphore _present_wait_semaphore,
									   vk::SwapchainKHR old_swapchain)
	{
		vk::PhysicalDevice ph_device = _device->GetPhysicalDevice();

		auto [surface_caps_res, surface_caps] = ph_device.getSurfaceCapabilitiesKHR(_surface->GetSurface());
		if(surface_caps_res != vk::Result::eSuccess)
			return {UnexpectedVkResult(surface_caps_res)};

		vk::Extent2D choosed_extent = _surface->ClampExtent(surface_caps, resolution);

		auto [surface_formats_res, surface_formats] = ph_device.getSurfaceFormatsKHR(_surface->GetSurface());
		if(surface_formats_res != vk::Result::eSuccess)
			return {UnexpectedVkResult(surface_formats_res)};

		vk::SurfaceFormatKHR choosed_swapchain_format = surface_formats[0];

		if(!_surface->IsImageUsageSupported(surface_caps, SWAPCHAIN_USAGE))
			return {UnexpectedSwapchainCreationResult(SwapchainCreationResult::UnsupportedImageUsage)};

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
										SWAPCHAIN_USAGE,
										vk::SharingMode::eExclusive,
										{},
										vk::SurfaceTransformFlagBitsKHR::eIdentity,
										composite_alpha,
										swapchain_present_mode,
										VK_TRUE,
										old_swapchain);

		Swapchain tmp_swapchain(_device);
		auto recreate_res = tmp_swapchain.Recreate(info,
												   _acquire_signal_semaphore,
												   _present_wait_semaphore);

		if(recreate_res != vk::Result::eSuccess)
			return {UnexpectedVkResult(recreate_res)};

		return tmp_swapchain;
	}
};
