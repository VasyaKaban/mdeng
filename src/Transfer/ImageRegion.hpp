/**
 * @file
 *
 * @Represents TransferImageRegions class
 */

#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "Region.hpp"
#include "Data.hpp"

namespace FireLand
{
	/**
	 * @brief The ImageRegion class
	 *
	 * Helpful structure as copy structure for high-level transfer class
	 */
	struct ImageRegion
	{
		vk::BufferImageCopy copy;///<copy structure without source offset
		std::size_t data_offset;///<offset within data pointer
	};

	/**
	 * @brief The ImageRegions using
	 *
	 * Small optimization variant for using one RegionOffset without dynamic allocations
	 */
	using ImageRegions = std::variant<RegionOffset<vk::BufferImageCopy>, RegionsOffsets<vk::BufferImageCopy>>;

	/**
	 * @brief The TransferImageRegions class
	 *
	 * Contains information about image to transfer, regions, image format and data
	 */
	struct TransferImageRegions
	{
		vk::Image image;///<image to transfer
		ImageRegions regions;///<transfer regions
		vk::DeviceSize texel_size;///<image texel size
		vk::ImageLayout layout;///<image layout
		mutable Data data;///data

		constexpr TransferImageRegions(vk::Image &_image,
									   vk::ImageLayout _layout,
									   vk::DeviceSize _texel_size,
									   std::vector<vk::BufferImageCopy> &&_regions,
									   std::vector<std::size_t> &&_regions_offsets,
									   Data _data) noexcept
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
									   Data _data) noexcept
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
			data = std::move(regs.data);
			regs.image = nullptr;
		}

		auto operator=(const TransferImageRegions &) -> TransferImageRegions & = default;
		constexpr auto operator=(TransferImageRegions &&regs) -> TransferImageRegions &
		{
			image = regs.image;
			regions = std::move(regs.regions);
			texel_size = regs.texel_size;
			layout = regs.layout;
			data = std::move(regs.data);
			regs.image = nullptr;

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

		void Transfer(vk::Buffer transfer_buffer,
					  std::byte *map_ptr,
					  vk::CommandBuffer command_buffer) const noexcept
		{
			std::visit([this, &transfer_buffer, map_ptr, &command_buffer]<typename BT>(const BT &val)
			{
				if constexpr(std::same_as<BT, RegionOffset<vk::BufferImageCopy>>)
				{
					const RegionOffset<vk::BufferImageCopy> &reg = val;
					memcpy(map_ptr + reg.region.bufferOffset, this->data.ptr + reg.offset, this->GetSize(0));

					command_buffer.copyBufferToImage(transfer_buffer,
													 this->image,
													 this->layout,
													 reg.region);
				}
				else
				{
					const RegionsOffsets<vk::BufferImageCopy> &regs = val;
					for(std::size_t i = 0; i < regs.regions.size(); i++)
						memcpy(map_ptr + regs.regions[i].bufferOffset,
							   this->data.ptr + regs.regions_offsets[i],
							   this->GetSize(i));

					command_buffer.copyBufferToImage(transfer_buffer,
													 this->image,
													 this->layout,
													 regs.regions);
				}
			}, regions);

			data.Delete();
		}
	};
};
