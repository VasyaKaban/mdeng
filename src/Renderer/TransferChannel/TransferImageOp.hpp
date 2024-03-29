#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include "../../hrs/debug.hpp"
#include "../../hrs/block.hpp"
#include "Data.h"

namespace FireLand
{
	struct TransferImageOpRegion
	{
		vk::ImageSubresourceLayers subresource_layers;
		vk::Extent3D image_extent;
		vk::DeviceSize data_offset;
		std::size_t data_index;

		constexpr TransferImageOpRegion(vk::ImageSubresourceLayers _subresource_layers,
										const vk::Extent3D &_image_extent,
										vk::DeviceSize _data_offset,
										std::size_t _data_index) noexcept
			: subresource_layers(_subresource_layers),
			  image_extent(_image_extent),
			  data_offset(_data_offset),
			  data_index(_data_index) {}
	};

	/*struct TransferImageOp
	{
		vk::Image dst_image;
		vk::DeviceSize block_size;
		vk::ImageLayout image_layout;
		std::variant<Data, std::vector<Data>> datas_var;
		std::variant<vk::BufferImageCopy, std::vector<vk::BufferImageCopy>> regions_var;

		TransferImageOp(vk::Image _dst_image,
						vk::DeviceSize _block_size,
						vk::ImageLayout _image_layout,
						Data &data,
						const TransferImageOpRegion &region,
						vk::DeviceSize &src_buffer_offset)
			: dst_image(_dst_image),
			  block_size(_block_size),
			  image_layout(_image_layout),
			  datas_var(data)
		{
			src_buffer_offset = hrs::round_up_size_to_alignment(src_buffer_offset, region.region_alignment);
			regions_var = vk::BufferImageCopy(src_buffer_offset, 0, 0,
											  region.subres_layers, {}, region.image_extent);
			src_buffer_offset += CalculateRegionSize(region);
		}

		TransferImageOp(vk::Image _dst_image,
						vk::DeviceSize _block_size,
						vk::ImageLayout _image_layout,
						std::vector<Data> &&_datas,
						const std::vector<TransferImageOpRegion> &_regions,
						vk::DeviceSize &src_buffer_offset)
			: dst_image(_dst_image),
			  block_size(_block_size),
			  image_layout(_image_layout),
			  datas_var(std::move(_datas))
		{
			hrs::assert_true_debug(std::get<std::vector<Data>>(datas_var).size() == _regions.size(),
								   "Sizes of datas and regions are not same!");

			std::vector<vk::BufferImageCopy> regions;
			regions.reserve(_regions.size());
			for(const auto &reg : _regions)
			{
				src_buffer_offset = hrs::round_up_size_to_alignment(src_buffer_offset, reg.region_alignment);
				regions.push_back(vk::BufferImageCopy(src_buffer_offset, 0, 0,
													  reg.subres_layers, {}, reg.image_extent));
				src_buffer_offset += CalculateRegionSize(reg);
			}
		}

		void Write(std::byte *src_buffer_ptr,
				   vk::Buffer src_buffer,
				   vk::CommandBuffer command_buffer) const noexcept
		{
			if(std::holds_alternative<vk::BufferImageCopy>(regions_var))
			{
				const Data &data = std::get<Data>(datas_var);
				const vk::BufferImageCopy &region = std::get<vk::BufferImageCopy>(regions_var);

				std::memcpy(src_buffer_ptr + region.bufferOffset,
							data.GetData(),
							CalculateRegionSize(region));

				command_buffer.copyBufferToImage(src_buffer, dst_image, image_layout, region);
			}
			else
			{
				const std::vector<Data> &datas = std::get<std::vector<Data>>(datas_var);
				const std::vector<vk::BufferImageCopy> &regions =
					std::get<std::vector<vk::BufferImageCopy>>(regions_var);

				for(std::size_t i = 0; i < datas.size(); i++)
				{
					std::memcpy(src_buffer_ptr + regions[i].bufferOffset,
								datas[i].GetData(),
								CalculateRegionSize(regions[i]));
				}

				command_buffer.copyBufferToImage(src_buffer, dst_image, image_layout, regions);
			}
		}

		vk::DeviceSize CalculateRegionSize(const TransferImageOpRegion &reg) const noexcept
		{
			return block_size * reg.image_extent.width * reg.image_extent.height * reg.image_extent.depth;
		}

		vk::DeviceSize CalculateRegionSize(const vk::BufferImageCopy &reg) const noexcept
		{
			return block_size * reg.imageExtent.width * reg.imageExtent.height * reg.imageExtent.depth;
		}
	};*/
};

