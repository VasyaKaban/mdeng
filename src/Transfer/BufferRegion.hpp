/**
 * @file
 *
 * Represents TransferBufferRegions class
 */

#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "Region.hpp"
#include "Data.hpp"

namespace FireLand
{
	/**
	 * @brief The BufferRegion class
	 *
	 * Helpful structure as copy structure for high-level transfer class
	 */
	struct BufferRegion
	{
		vk::BufferCopy copy;///<copy structure without source offset
		std::size_t data_offset;///<offset within data pointer
	};

	/**
	 * @brief The BufferRegions using
	 *
	 * Small optimization variant for using one RegionOffset without dynamic allocations
	 */
	using BufferRegions = std::variant<RegionOffset<vk::BufferCopy>, RegionsOffsets<vk::BufferCopy>>;

	/**
	 * @brief The TransferBufferRegions class
	 *
	 * Contains information about buffer to transfer, regions and data
	 */
	struct TransferBufferRegions
	{
		vk::Buffer buffer;///<buffer to transfer
		BufferRegions regions;///<transfer regions
		mutable Data data;///data

		/**
		 * @brief TransferBufferRegions
		 * @param _buffer buffer to transfer
		 * @param _regions array of vk::BufferCopy structures
		 * @param _regions_offsets array of data offsets per copy structure
		 * @param _data data to transfer
		 *
		 * Multiple regions version of constructor
		 */
		constexpr TransferBufferRegions(vk::Buffer &_buffer,
										std::vector<vk::BufferCopy> &&_regions,
										std::vector<std::size_t> &&_regions_offsets,
										Data _data) noexcept
		{
			buffer = _buffer;
			regions = RegionsOffsets(std::move(_regions), std::move(_regions_offsets));
			data = _data;
		}

		/**
		 * @brief TransferBufferRegions
		 * @param _buffer buffer to transfer
		 * @param _region vk::BufferCopy structure
		 * @param _regions_offset data offset for copy structure
		 * @param _data data to transfer
		 *
		 * One region version of constructor
		 */
		constexpr TransferBufferRegions(vk::Buffer &_buffer,
										const vk::BufferCopy &_region,
										std::size_t _region_offset,
										Data _data) noexcept
		{
			buffer = _buffer;
			regions = RegionOffset(_region, _region_offset);
			data = _data;
		}

		~TransferBufferRegions() = default;
		TransferBufferRegions(const TransferBufferRegions &) = default;
		TransferBufferRegions(TransferBufferRegions &&regs) noexcept
		{
			buffer = regs.buffer;
			regions = std::move(regs.regions);
			data = std::move(regs.data);
			regs.buffer = VK_NULL_HANDLE;
		}

		TransferBufferRegions & operator=(const TransferBufferRegions &) = default;
		TransferBufferRegions & operator=(TransferBufferRegions &&regs) noexcept
		{
			buffer = regs.buffer;
			regions = std::move(regs.regions);
			data = std::move(regs.data);
			regs.buffer = VK_NULL_HANDLE;

			return *this;
		}

		void Transfer(vk::Buffer transfer_buffer,
					  std::byte *map_ptr,
					  vk::CommandBuffer command_buffer) const noexcept
		{
			std::visit([this, map_ptr, &command_buffer, &transfer_buffer]<typename BT>(const BT &val)
			{
				if constexpr(std::same_as<BT, RegionOffset<vk::BufferCopy>>)
				{
					const RegionOffset<vk::BufferCopy> &reg = val;
					memcpy(map_ptr + reg.region.srcOffset, this->data.ptr + reg.offset, reg.region.size);

					command_buffer.copyBuffer(transfer_buffer, this->buffer, reg.region);
				}
				else
				{
					const RegionsOffsets<vk::BufferCopy> &regs = val;
					for(std::size_t i = 0; i < regs.regions.size(); i++)
						memcpy(map_ptr + regs.regions[i].srcOffset,
							   this->data.ptr + regs.regions_offsets[i],
							   regs.regions[i].size);

					command_buffer.copyBuffer(transfer_buffer, this->buffer, regs.regions);
				}
			}, regions);

			data.Delete();
		}
	};
};
