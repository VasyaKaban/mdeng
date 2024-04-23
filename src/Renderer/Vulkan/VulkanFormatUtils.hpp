#pragma once

#include <format>
#include "VulkanInclude.hpp"
#include "hrs/debug.hpp"
#include "hrs/instantiation.hpp"

namespace FireLand
{
	inline vk::Result IsFormatSatisfyRequirements(vk::PhysicalDevice ph_device,
												  vk::Format format = {},
												  vk::FormatFeatureFlags features = {},
												  vk::ImageType type = {},
												  vk::ImageTiling tiling = {},
												  vk::ImageUsageFlags usage = {},
												  const vk::Extent3D &extent = {},
												  std::uint32_t mip_levels = 1,
												  std::uint32_t array_layers = 1,
												  vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1,
												  vk::ImageCreateFlags create_flags = {})
	{
		hrs::assert_true_debug(ph_device, "Physical device isn't created yet!");

		auto format_props = ph_device.getFormatProperties(format);
		if(tiling == vk::ImageTiling::eLinear)
		{
			if(!(format_props.linearTilingFeatures & features))
				return vk::Result::eErrorFormatNotSupported;
		}
		else if(tiling == vk::ImageTiling::eOptimal)
		{
			if(!(format_props.optimalTilingFeatures & features))
				return vk::Result::eErrorFormatNotSupported;
		}

		auto [image_format_props_res, image_format_props] =
			ph_device.getImageFormatProperties(format,
											   type,
											   tiling,
											   usage,
											   create_flags);

		if(image_format_props_res != vk::Result::eSuccess)
			return image_format_props_res;

		if(image_format_props.maxExtent < extent)
			return vk::Result::eErrorFormatNotSupported;
		if(image_format_props.maxArrayLayers < array_layers)
			return vk::Result::eErrorFormatNotSupported;
		if(image_format_props.maxMipLevels < mip_levels)
			return vk::Result::eErrorFormatNotSupported;
		if(image_format_props.sampleCounts < samples)
			return vk::Result::eErrorFormatNotSupported;

		return vk::Result::eSuccess;
	}

	constexpr bool IsDepthStencilFormat(vk::Format format) noexcept
	{
		switch(format)
		{
			case vk::Format::eD16UnormS8Uint:
			case vk::Format::eD24UnormS8Uint:
			case vk::Format::eD32SfloatS8Uint:
				return true;
				break;
			default:
				return false;
				break;
		}
	}

	constexpr bool IsDepthOnlyFormat(vk::Format format) noexcept
	{
		switch(format)
		{
			case vk::Format::eD16Unorm:
			case vk::Format::eX8D24UnormPack32:
			case vk::Format::eD32Sfloat:
				return true;
				break;
			default:
				return false;
				break;
		}
	}
};
