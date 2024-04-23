#include "Surface.h"
#include "Context.h"
#include "hrs/debug.hpp"
#include <ranges>

namespace FireLand
{
	Surface::Surface(const Context *_parent_context, vk::SurfaceKHR _surface) noexcept
		: parent_context(_parent_context),
		  surface(_surface) {}

	Surface::Surface(Surface &&sur) noexcept
		: parent_context(sur.parent_context),
		  surface(std::exchange(sur.surface, VK_NULL_HANDLE)) {}

	Surface & Surface::operator=(Surface &&sur) noexcept
	{
		Destroy();

		parent_context = std::exchange(sur.parent_context, VK_NULL_HANDLE);
		surface = sur.surface;

		return *this;
	}

	void Surface::Destroy()
	{
		if(IsCreated())
		{
			parent_context->GetHandle().destroy(surface);
			surface = VK_NULL_HANDLE;
		}
	}

	vk::SurfaceKHR Surface::GetHandle() const noexcept
	{
		return surface;
	}

	const Context * Surface::GetContext() const noexcept
	{
		return parent_context;
	}

	bool Surface::IsCreated() const noexcept
	{
		return surface;
	}

	hrs::expected<bool, vk::Result>
	Surface::IsSurfaceSupported(vk::PhysicalDevice ph_device, std::uint32_t queue_family_index) const noexcept
	{
		hrs::assert_true_debug(ph_device, "Physical device isn't created yet!");
		if(!IsCreated())
			return false;

		auto [is_supported_res, is_supported] = ph_device.getSurfaceSupportKHR(queue_family_index, surface);
		if(is_supported_res != vk::Result::eSuccess)
			return is_supported_res;

		return is_supported;
	}

	hrs::expected<bool, vk::Result>
	Surface::IsPresentModeSupported(vk::PhysicalDevice ph_device, vk::PresentModeKHR mode) const
	{
		hrs::assert_true_debug(ph_device, "Physical device isn't created yet!");
		if(!IsCreated())
			return false;

		auto [modes_res, modes] = ph_device.getSurfacePresentModesKHR(surface);
		if(modes_res != vk::Result::eSuccess)
			return modes_res;

		const auto it = std::ranges::find(modes, mode);
		return it != modes.end();
	}

	vk::Extent2D Surface::ClampExtent(const vk::SurfaceCapabilitiesKHR &caps,
									  vk::Extent2D extent) noexcept
	{
		if(caps.currentExtent.width == caps.currentExtent.height && caps.currentExtent.width == 0xFFFFFFFF)
			return caps.currentExtent;

		return vk::Extent2D(std::clamp(extent.width, caps.minImageExtent.width, caps.maxImageExtent.width),
							std::clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height));
	}

	std::uint32_t Surface::ClampImageCount(const vk::SurfaceCapabilitiesKHR &caps,
										   std::uint32_t image_count) noexcept
	{
		const std::uint32_t max_count = (caps.maxImageCount == 0 ?
											 std::numeric_limits<std::uint32_t>::max() :
											 caps.maxImageCount);

		return std::clamp(image_count, caps.minImageCount, max_count);
	}

	std::uint32_t Surface::ClampLayerCount(const vk::SurfaceCapabilitiesKHR &caps,
										   std::uint32_t layer_count) noexcept
	{
		return std::clamp(layer_count, (std::uint32_t)1, caps.maxImageArrayLayers);
	}

	bool Surface::IsTransformSupported(const vk::SurfaceCapabilitiesKHR &caps,
									   vk::SurfaceTransformFlagsKHR transform) noexcept
	{
		return static_cast<bool>(caps.supportedTransforms & transform);
	}

	bool Surface::IsCompositeAlphaSupported(const vk::SurfaceCapabilitiesKHR &caps,
											vk::CompositeAlphaFlagBitsKHR alpha) noexcept
	{
		return static_cast<bool>(caps.supportedCompositeAlpha & alpha);
	}

	bool Surface::IsImageUsageSupported(const vk::SurfaceCapabilitiesKHR &caps,
										vk::ImageUsageFlags usage) noexcept
	{
		return static_cast<bool>(caps.supportedUsageFlags & usage);
	}
};
