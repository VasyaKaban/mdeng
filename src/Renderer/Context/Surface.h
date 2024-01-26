#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include "../../hrs/non_creatable.hpp"
#include "../../hrs/expected.hpp"
#include <span>

namespace FireLand
{
	class Surface : public hrs::non_copyable, public hrs::non_move_assignable
	{
	public:
		Surface(vk::SurfaceKHR _surface) noexcept;
		~Surface() = default;
		Surface(Surface &&sur) noexcept;

		void Destroy(vk::Instance instance);

		vk::SurfaceKHR GetSurface() const noexcept;

		bool IsCreated() const noexcept;

		hrs::expected<bool, vk::Result>
		IsSurfaceSupported(vk::PhysicalDevice ph_device, std::uint32_t queue_family_index) const noexcept;

		hrs::expected<bool, vk::Result>
		IsPresentModeSupported(vk::PhysicalDevice ph_device, vk::PresentModeKHR mode) const;

		vk::Extent2D ClampExtent(const vk::SurfaceCapabilitiesKHR &caps, vk::Extent2D extent) const noexcept;
		std::uint32_t ClampImageCount(const vk::SurfaceCapabilitiesKHR &caps,
									  std::uint32_t image_count) const noexcept;
		std::uint32_t ClampLayerCount(const vk::SurfaceCapabilitiesKHR &caps,
									  std::uint32_t layer_count) const noexcept;
		bool IsTransformSupported(const vk::SurfaceCapabilitiesKHR &caps,
								  vk::SurfaceTransformFlagsKHR transform) const noexcept;
		bool IsCompositeAlphaSupported(const vk::SurfaceCapabilitiesKHR &caps,
									   vk::CompositeAlphaFlagBitsKHR alpha) const noexcept;
		bool IsImageUsageSupported(const vk::SurfaceCapabilitiesKHR &caps,
								   vk::ImageUsageFlags usage) const noexcept;

	private:
		vk::SurfaceKHR surface;
	};
};
