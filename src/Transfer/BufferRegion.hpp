#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "Region.hpp"

namespace FireLand
{
	struct BufferRegion
	{
		vk::BufferCopy copy;
		std::size_t data_offset;
	};

	using BufferRegions = std::variant<RegionOffset<vk::BufferCopy>, RegionsOffsets<vk::BufferCopy>>;

	struct TransferBufferRegions
	{
		vk::Buffer buffer;
		BufferRegions regions;
		std::byte *data;

		constexpr TransferBufferRegions(vk::Buffer &_buffer,
										std::vector<vk::BufferCopy> &&_regions,
										std::vector<std::size_t> &&_regions_offsets,
										std::byte *_data) noexcept
		{
			buffer = _buffer;
			regions = RegionsOffsets(std::move(_regions), std::move(_regions_offsets));
			data = _data;
		}

		constexpr TransferBufferRegions(vk::Buffer &_buffer,
										const vk::BufferCopy &_region,
										std::size_t _region_offset,
										std::byte *_data) noexcept
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
			data = regs.data;
			regs.buffer = nullptr;
			regs.data = nullptr;
		}

		auto operator=(const TransferBufferRegions &) -> TransferBufferRegions & = default;
		constexpr auto operator=(TransferBufferRegions &&regs) noexcept -> TransferBufferRegions &
		{
			buffer = regs.buffer;
			regions = std::move(regs.regions);
			data = regs.data;
			regs.buffer = nullptr;
			regs.data = nullptr;
			return *this;
		}
	};
};
