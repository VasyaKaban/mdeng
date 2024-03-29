#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include "../../hrs/debug.hpp"
#include "../../hrs/block.hpp"
#include "Data.h"

namespace FireLand
{
	struct TransferBufferOpRegion
	{
		hrs::block<vk::DeviceSize> data_blk;
		vk::DeviceSize dst_buffer_offset;
		std::size_t data_index;

		TransferBufferOpRegion(const hrs::block<vk::DeviceSize> &_data_blk,
							   vk::DeviceSize _dst_buffer_offset,
							   std::size_t _data_index) noexcept
			: data_blk(_data_blk),
			  dst_buffer_offset(_dst_buffer_offset),
			  data_index(_data_index) {}
	};

	/*struct TransferBufferOp
	{
		vk::Buffer dst_buffer;
		std::variant<Data, std::vector<Data>> datas_var;
		std::variant<vk::BufferCopy, std::vector<vk::BufferCopy>> regions_var;

		TransferBufferOp(vk::Buffer _dst_buffer,
						 Data data,
						 const TransferBufferOpRegion &region,
						 vk::DeviceSize &src_buffer_offset)
			: dst_buffer(_dst_buffer),
			  datas_var(data)
		{
			src_buffer_offset = hrs::round_up_size_to_alignment(src_buffer_offset, region.region_alignment);
			regions_var = vk::BufferCopy(src_buffer_offset, region.dst_buffer_offset, region.data_size);
			src_buffer_offset += region.data_size;
		}

		TransferBufferOp(vk::Buffer _dst_buffer,
						 std::vector<Data> &&_datas,
						 const std::vector<TransferBufferOpRegion> &_regions,
						 vk::DeviceSize &src_buffer_offset)
			: dst_buffer(_dst_buffer),
			  datas_var(std::move(_datas))
		{
			hrs::assert_true_debug(std::get<std::vector<Data>>(datas_var).size() == _regions.size(),
								   "Sizes of datas and regions are not same!");

			std::vector<vk::BufferCopy> regions;
			regions.reserve(_regions.size());
			for(const auto &reg : _regions)
			{
				src_buffer_offset = hrs::round_up_size_to_alignment(src_buffer_offset, reg.region_alignment);
				regions.push_back(vk::BufferCopy(src_buffer_offset, reg.dst_buffer_offset, reg.data_size));
				src_buffer_offset += reg.data_size;
			}

			regions_var = std::move(regions);
		}

		void Write(std::byte *src_buffer_ptr,
				   vk::Buffer src_buffer,
				   vk::CommandBuffer command_buffer) const noexcept
		{
			if(std::holds_alternative<vk::BufferCopy>(regions_var))
			{
				const Data &data = std::get<Data>(datas_var);
				const vk::BufferCopy &region = std::get<vk::BufferCopy>(regions_var);

				std::memcpy(src_buffer_ptr + region.srcOffset,
							data.GetData(),
							region.size);

				command_buffer.copyBuffer(src_buffer, dst_buffer, region);
			}
			else
			{
				const std::vector<Data> &datas = std::get<std::vector<Data>>(datas_var);
				const std::vector<vk::BufferCopy> &regions = std::get<std::vector<vk::BufferCopy>>(regions_var);

				for(std::size_t i = 0; i < datas.size(); i++)
				{
					std::memcpy(src_buffer_ptr + regions[i].srcOffset,
								datas[i].GetData(),
								regions[i].size);
				}

				command_buffer.copyBuffer(src_buffer, dst_buffer, regions);
			}
		}
	};*/
};
