#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "hrs/non_creatable.hpp"
#include "hrs/expected.hpp"
#include <span>
#include <cstdint>

namespace FireLand
{
	class Context;

	class Surface : public hrs::non_copyable
	{
	public:
		Surface(const Context *_parent_context, vk::SurfaceKHR _surface) noexcept;
		~Surface() = default;
		Surface(Surface &&sur) noexcept;
		Surface & operator=(Surface &&sur) noexcept;

		void Destroy();

		vk::SurfaceKHR GetHandle() const noexcept;
		const Context * GetContext() const noexcept;

		bool IsCreated() const noexcept;

		hrs::expected<bool, vk::Result>
		IsSurfaceSupported(vk::PhysicalDevice ph_device, std::uint32_t queue_family_index) const noexcept;

		hrs::expected<bool, vk::Result>
		IsPresentModeSupported(vk::PhysicalDevice ph_device, vk::PresentModeKHR mode) const;

		static vk::Extent2D ClampExtent(const vk::SurfaceCapabilitiesKHR &caps,
										vk::Extent2D extent)noexcept;

		static std::uint32_t ClampImageCount(const vk::SurfaceCapabilitiesKHR &caps,
											 std::uint32_t image_count)noexcept;

		static std::uint32_t ClampLayerCount(const vk::SurfaceCapabilitiesKHR &caps,
											 std::uint32_t layer_count)noexcept;

		static bool IsTransformSupported(const vk::SurfaceCapabilitiesKHR &caps,
										 vk::SurfaceTransformFlagsKHR transform) noexcept;

		static bool IsCompositeAlphaSupported(const vk::SurfaceCapabilitiesKHR &caps,
											  vk::CompositeAlphaFlagBitsKHR alpha) noexcept;

		static bool IsImageUsageSupported(const vk::SurfaceCapabilitiesKHR &caps,
										  vk::ImageUsageFlags usage) noexcept;

	private:
		const Context *parent_context = {};
		vk::SurfaceKHR surface;
	};
};
