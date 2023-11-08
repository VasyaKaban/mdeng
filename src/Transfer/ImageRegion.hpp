#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "Region.hpp"

namespace FireLand
{
	struct ImageRegion
	{
		vk::BufferImageCopy copy;
		std::size_t data_offset;
	};

	using ImageRegions = std::variant<RegionOffset<vk::BufferImageCopy>, RegionsOffsets<vk::BufferImageCopy>>;

	struct TransferImageRegions
	{
		vk::Image image;
		ImageRegions regions;
		vk::DeviceSize texel_size;
		vk::ImageLayout layout;
		std::byte *data;

		constexpr TransferImageRegions(vk::Image &_image,
									   vk::ImageLayout _layout,
									   vk::DeviceSize _texel_size,
									   std::vector<vk::BufferImageCopy> &&_regions,
									   std::vector<std::size_t> &&_regions_offsets,
									   std::byte *_data) noexcept
		{
			image = _image;
			regions = RegionsOffsets(std::move(_regions), std::move(_regions_offsets));
			texel_size = _texel_size;
			layout = _layout;
			data = _data;
		}

		constexpr TransferImageRegions(vk::Image &_image,
									   vk::ImageLayout _layout,
									   vk::DeviceSize _texel_size,
									   const vk::BufferImageCopy &_region,
									   std::size_t _region_offset,
									   std::byte *_data) noexcept
		{
			image = _image;
			regions = RegionOffset(_region, _region_offset);
			texel_size = _texel_size;
			layout = _layout;
			data = _data;
		}

		~TransferImageRegions() = default;
		TransferImageRegions(const TransferImageRegions &) = default;
		TransferImageRegions(TransferImageRegions &&regs) noexcept
		{
			image = regs.image;
			regions = std::move(regs.regions);
			texel_size = regs.texel_size;
			layout = regs.layout;
			data = regs.data;
			regs.image = nullptr;
			regs.data = nullptr;
		}

		auto operator=(const TransferImageRegions &) -> TransferImageRegions & = default;
		constexpr auto operator=(TransferImageRegions &&regs) -> TransferImageRegions &
		{
			image = regs.image;
			regions = std::move(regs.regions);
			texel_size = regs.texel_size;
			layout = regs.layout;
			data = regs.data;
			regs.image = nullptr;
			regs.data = nullptr;
			return *this;
		}

		constexpr auto GetSize(std::size_t ind) const noexcept -> vk::DeviceSize
		{
			if(std::holds_alternative<RegionsOffsets<vk::BufferImageCopy>>(regions))
			{
				const auto regions_offsets = std::get<RegionsOffsets<vk::BufferImageCopy>>(regions);
				return regions_offsets.regions[ind].imageExtent.width *
					   regions_offsets.regions[ind].imageExtent.height *
					   regions_offsets.regions[ind].imageExtent.depth *
					   texel_size;
			}
			else
			{
				const auto region_offset = std::get<RegionOffset<vk::BufferImageCopy>>(regions);
				return region_offset.region.imageExtent.width *
					   region_offset.region.imageExtent.height *
					   region_offset.region.imageExtent.depth *
					   texel_size;
			}
		}

		constexpr static auto GetSize(const vk::BufferImageCopy &copy, vk::DeviceSize texel_size) -> vk::DeviceSize
		{
			return copy.imageExtent.width *
				   copy.imageExtent.height *
				   copy.imageExtent.depth *
				   texel_size;
		}
	};
};
