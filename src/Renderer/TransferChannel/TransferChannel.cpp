#include "TransferChannel.h"
#include "../Context/Device.h"
#include "../Allocator/AllocateFromMany.hpp"
#include "TransferBufferOp.hpp"

namespace FireLand
{
	void TransferChannel::init(BoundedBufferSizeFillness &&buffer,
							   vk::Fence _wait_fence,
							   vk::Queue _transfer_queue,
							   vk::CommandBuffer _command_buffer,
							   const std::function<NewPoolSizeCalculator> &_calc,
							   vk::DeviceSize _rounding_size,
							   vk::DeviceSize _buffer_size_power,
							   vk::DeviceSize _buffer_alignment) noexcept
	{
		buffers.push_back(std::move(buffer));
		wait_fence = _wait_fence;
		transfer_queue = _transfer_queue;
		calc = _calc;
		rounding_size = _rounding_size;
		buffer_size_power = _buffer_size_power;
		buffer_alignment = _buffer_alignment;
		is_in_write = false;
		command_buffer = _command_buffer;
	}

	TransferChannel::TransferChannel(Device *_parent_device) noexcept
		: parent_device(_parent_device) {}

	TransferChannel::~TransferChannel()
	{
		Destroy();
	}

	TransferChannel::TransferChannel(TransferChannel &&tc) noexcept
		: parent_device(tc.parent_device),
		  buffers(std::move(tc.buffers)),
		  wait_fence(tc.wait_fence),
		  transfer_queue(tc.transfer_queue),
		  calc(tc.calc),
		  rounding_size(tc.rounding_size),
		  buffer_alignment(tc.buffer_alignment),
		  buffer_size_power(tc.buffer_size_power),
		  is_in_write(tc.is_in_write),
		  command_buffer(tc.command_buffer) {}

	TransferChannel & TransferChannel::operator=(TransferChannel &&tc) noexcept
	{
		Destroy();

		parent_device = tc.parent_device;
		buffers = std::move(tc.buffers);
		wait_fence = tc.wait_fence;
		transfer_queue = tc.transfer_queue;
		calc = tc.calc;
		rounding_size = tc.rounding_size;
		buffer_alignment = tc.buffer_alignment;
		buffer_size_power = tc.buffer_size_power;
		is_in_write = tc.is_in_write;
		command_buffer = tc.command_buffer;

		return *this;
	}

	hrs::error TransferChannel::Recreate(vk::Queue _transfer_queue,
										 vk::CommandBuffer _command_buffer,
										 vk::DeviceSize _rounding_size,
										 std::size_t _buffer_size_power,
										 vk::DeviceSize _buffer_alignment,
										 const std::function<NewPoolSizeCalculator> &_calc)
	{
		hrs::assert_true_debug(_rounding_size > 0, "Rounding size must be greater than zero!");

		Destroy();

		constexpr static vk::FenceCreateInfo fence_info(vk::FenceCreateFlagBits::eSignaled);
		auto [u_fence_res, u_fence] = parent_device->GetHandle().createFenceUnique(fence_info);
		if(u_fence_res != vk::Result::eSuccess)
			return u_fence_res;

		if(_buffer_size_power == 0)
		{
			init({},
				 u_fence.release(),
				 _transfer_queue,
				 _command_buffer,
				 _calc,
				 _rounding_size,
				 _buffer_size_power,
				 buffer_alignment);
			return {};
		}

		auto buffer_exp = allocate_buffer({_rounding_size * _buffer_size_power, _buffer_alignment}, _calc);
		if(!buffer_exp)
			return buffer_exp.error();

		init(std::move(buffer_exp.value()),
			 u_fence.release(),
			 _transfer_queue,
			 _command_buffer,
			 _calc,
			 _rounding_size,
			 _buffer_size_power,
			 _buffer_alignment);

		return {};
	}

	vk::CommandBuffer TransferChannel::GetCommandBuffer() const noexcept
	{
		return command_buffer;
	}

	void TransferChannel::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		vk::Result res = WaitFence();
		hrs::assert_true(res == vk::Result::eSuccess, "BAD WAIT RESULT!!!");

		vk::Device device_handle = parent_device->GetHandle();
		wait_fence = (device_handle.destroy(wait_fence), VK_NULL_HANDLE);
		free_buffers();
		buffers.clear();
	}

	bool TransferChannel::IsCreated() const noexcept
	{
		return !buffers.empty();
	}

	vk::Result TransferChannel::Flush(std::span<const vk::SubmitInfo> submits) noexcept
	{
		if(!is_in_write)
			return vk::Result::eSuccess;

		vk::Result res = command_buffer.end();
		if(res != vk::Result::eSuccess)
			return res;

		res = parent_device->GetHandle().resetFences(wait_fence);
		if(res != vk::Result::eSuccess)
			return res;

		res = transfer_queue.submit(submits, wait_fence);
		if(res != vk::Result::eSuccess)
			return res;

		return vk::Result::eSuccess;
	}

	vk::Result TransferChannel::GetWaitFenceStatus() const noexcept
	{
		return parent_device->GetHandle().getFenceStatus(wait_fence);
	}

	vk::Result TransferChannel::PlainWaitFence(std::uint64_t timeout) const noexcept
	{
		return parent_device->GetHandle().waitForFences(wait_fence, VK_TRUE, timeout);
	}

	vk::Result TransferChannel::WaitFence(std::uint64_t timeout) noexcept
	{
		vk::Result res = PlainWaitFence(timeout);
		for(auto &wait_func : pending_wait_functions)
			wait_func();

		pending_wait_functions.clear();
		return res;
	}

	hrs::expected<std::size_t, hrs::error>
	TransferChannel::CopyBuffer(vk::Buffer dst_buffer,
								std::span<const Data> datas,
								std::span<const TransferBufferOpRegion> regions)
	{
		if(datas.empty() || regions.empty())
			return 0;

		vk::DeviceSize common_size = 0;
		for(const auto &reg : regions)
			common_size += reg.data_blk.size;

		for(std::size_t i = 0; i < buffers.size(); i++)
		{
			auto &buffer = buffers[i];
			if(buffer.IsEnoughSpace(common_size))
			{
				if(!IsWriteStarted())
				{
					vk::Result res = start_write();
					if(res != vk::Result::eSuccess)
						return res;
				}

				copy_buffer(dst_buffer, datas, regions, buffer);
				return i;
			}
		}

		auto unexp_res = push_new_buffer(common_size);
		if(unexp_res)
			return unexp_res;

		copy_buffer(dst_buffer, datas, regions, buffers.back());
		return buffers.size() - 1;
	}

	hrs::expected<std::size_t, hrs::error>
	TransferChannel::CopyImageSubresource(vk::Image dst_image,
										  vk::ImageLayout image_layout,
										  vk::DeviceSize block_size,
										  std::span<const Data> datas,
										  std::span<const TransferImageOpRegion> regions)
	{
		if(datas.empty() || regions.empty() || block_size == 0)
			return 0;

		vk::DeviceSize common_size = 0;
		for(const auto &reg : regions)
			common_size += calculate_region_size(reg.image_extent, block_size);

		for(std::size_t i = 0; i < buffers.size(); i++)
		{
			auto &buffer = buffers[i];
			if(buffer.IsEnoughSpace(common_size))
			{
				if(!IsWriteStarted())
				{
					vk::Result res = start_write();
					if(res != vk::Result::eSuccess)
						return res;
				}

				copy_image(dst_image, image_layout, block_size, datas, regions, buffer);
				return i;
			}
		}

		auto unexp_res = push_new_buffer(common_size);
		if(unexp_res)
			return unexp_res;

		copy_image(dst_image, image_layout, block_size, datas, regions, buffers.back());
		return buffers.size() - 1;
	}

	vk::Result TransferChannel::EmbedBarrier(vk::PipelineStageFlags src_stages,
											 vk::PipelineStageFlags dst_stages,
											 vk::DependencyFlags dependency,
											 std::span<const vk::MemoryBarrier> memory_barriers,
											 std::span<const vk::BufferMemoryBarrier> buffer_memory_barriers,
											 std::span<const vk::ImageMemoryBarrier> image_memory_barriers)
	{
		if(!IsWriteStarted())
		{
			vk::Result res = start_write();
			if(res != vk::Result::eSuccess)
				return res;
		}

		command_buffer.pipelineBarrier(src_stages,
									   dst_stages,
									   dependency,
									   memory_barriers,
									   buffer_memory_barriers,
									   image_memory_barriers);

		return vk::Result::eSuccess;
	}

	bool TransferChannel::IsWriteStarted() const noexcept
	{
		return is_in_write;
	}

	Device * TransferChannel::GetParentDevice () noexcept
	{
		return parent_device;
	}

	const Device * TransferChannel::GetParentDevice () const noexcept
	{
		return parent_device;
	}

	vk::Queue TransferChannel::GetTransferQueue() const noexcept
	{
		return transfer_queue;
	}

	const std::function<NewPoolSizeCalculator> & TransferChannel::GetNewPoolSizeCalculator() const noexcept
	{
		return calc;
	}

	vk::DeviceSize TransferChannel::GetRoundingSize() const noexcept
	{
		return rounding_size;
	}

	vk::DeviceSize TransferChannel::GetBufferAlignment() const noexcept
	{
		return buffer_alignment;
	}

	vk::Fence TransferChannel::GetFence() const noexcept
	{
		return wait_fence;
	}

	const BoundedBufferSize & TransferChannel::GetBuffer(std::size_t index) const noexcept
	{
		return buffers[index].bounded_buffer_size;
	}

	hrs::error TransferChannel::FlattenBuffers(std::size_t lower_buffer_size_power,
											   std::size_t higher_buffer_size_power)
	{
		if(lower_buffer_size_power > higher_buffer_size_power)
			return {};

		vk::Result res = WaitFence();
		if(res != vk::Result::eSuccess)
			return res;


		if(higher_buffer_size_power == 0)
		{
			free_buffers();
			buffers.clear();
		}
		else
		{
			std::size_t target_power = std::numeric_limits<std::size_t>::max();
			std::optional<std::size_t> found_i = {};
			for(std::size_t i = 0; i < buffers.size(); i++)
			{
				std::size_t buffer_power = buffers[i].bounded_buffer_size.size / rounding_size;
				if(buffer_power >= lower_buffer_size_power &&
					buffer_power <= higher_buffer_size_power &&
					buffer_power < target_power)
				{
					found_i = i;
				}
			}

			if(found_i)
			{
				auto buffer = std::move(buffers[*found_i]);
				free_buffers();
				buffers.resize(1);
				buffers[0] = std::move(buffer);
			}
			else
			{
				free_buffers();
				auto buffer_exp = allocate_buffer({lower_buffer_size_power, buffer_alignment}, calc);
				if(!buffer_exp)
					return buffer_exp.error();

				buffers.resize(1);
				buffers[0] = std::move(buffer_exp.value());
			}
		}

		return {};
	}

	hrs::expected<BoundedBufferSize, hrs::error>
	TransferChannel::allocate_buffer(const hrs::mem_req<vk::DeviceSize> &req,
									 const std::function<NewPoolSizeCalculator> &_calc)
	{
		constexpr static std::array variants =
		{
			MemoryPropertyOpFlags(vk::MemoryPropertyFlagBits::eHostVisible,
								  MemoryTypeSatisfyOp::Any,
								  AllocationFlags::MapMemory),
			MemoryPropertyOpFlags(vk::MemoryPropertyFlagBits::eHostVisible,
								  MemoryTypeSatisfyOp::Any,
								  hrs::flags(AllocationFlags::MapMemory) |
									  AllocationFlags::AllowPlaceWithMixedResources)
		};

		const vk::BufferCreateInfo info({},
										req.size,
										vk::BufferUsageFlagBits::eTransferSrc,
										vk::SharingMode::eExclusive);

		auto buffer_exp = AllocateFromMany(*parent_device->GetAllocator(),
										   variants,
										   info,
										   req.alignment,
										   calc);

		if(!buffer_exp)
			return buffer_exp.error();

		return BoundedBufferSize{std::move(buffer_exp.value()), req.size};
	}

	vk::Result TransferChannel::start_write() noexcept
	{
		vk::Result res = WaitFence();
		if(res != vk::Result::eSuccess)
			return res;

		constexpr static vk::CommandBufferBeginInfo info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		res = command_buffer.begin(info);
		if(res != vk::Result::eSuccess)
			return res;

		is_in_write = true;
		return vk::Result::eSuccess;
	}

	void TransferChannel::copy_buffer(vk::Buffer dst_buffer,
									  std::span<const Data> datas,
									  std::span<const TransferBufferOpRegion> regions,
									  BoundedBufferSizeFillness &buffer)
	{
		if(regions.size() == 1)
		{
			const auto &region = regions[0];
			const vk::BufferCopy copy(buffer.fillness,
									  region.dst_buffer_offset,
									  region.data_blk.size);

			std::memcpy(buffer.bounded_buffer_size.bounded_buffer.GetBufferMapPtr() + buffer.fillness,
						datas[region.data_index].GetData() + region.data_blk.offset,
						region.data_blk.size);

			buffer.fillness += region.data_blk.size;
			command_buffer.copyBuffer(buffer.bounded_buffer_size.bounded_buffer.buffer,
									  dst_buffer,
									  copy);
		}
		else
		{
			std::vector<vk::BufferCopy> copy_regions;
			copy_regions.reserve(regions.size());
			for(const auto &region : regions)
			{
				const Data &data = datas[region.data_index];
				std::memcpy(buffer.bounded_buffer_size.bounded_buffer.GetBufferMapPtr() + buffer.fillness,
							data.GetData() + region.data_blk.offset,
							region.data_blk.size);

				copy_regions.push_back(vk::BufferCopy{buffer.fillness,
													  region.dst_buffer_offset,
													  region.data_blk.size});

				buffer.fillness += region.data_blk.size;
			}

			command_buffer.copyBuffer(buffer.bounded_buffer_size.bounded_buffer.buffer,
									  dst_buffer,
									  copy_regions);
		}
	}

	void TransferChannel::copy_image(vk::Image dst_image,
									 vk::ImageLayout image_layout,
									 vk::DeviceSize block_size,
									 std::span<const Data> datas,
									 std::span<const TransferImageOpRegion> regions,
									 BoundedBufferSizeFillness &buffer)
	{
		if(regions.size() == 1)
		{
			const auto &region = regions[0];
			vk::DeviceSize size = calculate_region_size(region.image_extent, block_size);
			const vk::BufferImageCopy copy(buffer.fillness,
										   0,
										   0,
										   region.subresource_layers,
										   {0, 0, 0},
										   region.image_extent);

			std::memcpy(buffer.bounded_buffer_size.bounded_buffer.GetBufferMapPtr() + buffer.fillness,
						datas[region.data_index].GetData() + region.data_offset,
						size);

			buffer.fillness += size;
			command_buffer.copyBufferToImage(buffer.bounded_buffer_size.bounded_buffer.buffer,
											 dst_image,
											 image_layout,
											 copy);
		}
		else
		{
			std::vector<vk::BufferImageCopy> copy_regions;
			copy_regions.reserve(regions.size());
			for(const auto &region : regions)
			{
				const Data &data = datas[region.data_index];
				vk::DeviceSize size = calculate_region_size(region.image_extent, block_size);

				std::memcpy(buffer.bounded_buffer_size.bounded_buffer.GetBufferMapPtr() + buffer.fillness,
							datas[region.data_index].GetData() + region.data_offset,
							size);

				copy_regions.push_back(vk::BufferImageCopy(buffer.fillness,
														   0,
														   0,
														   region.subresource_layers,
														   {0, 0, 0},
														   region.image_extent));
				buffer.fillness += size;
			}

			command_buffer.copyBufferToImage(buffer.bounded_buffer_size.bounded_buffer.buffer,
											 dst_image,
											 image_layout,
											 copy_regions);
		}
	}

	vk::DeviceSize TransferChannel::calculate_region_size(const vk::Extent3D &extent,
														  vk::DeviceSize block_size) const noexcept
	{
		return extent.width * extent.height * extent.depth * block_size;
	}

	void TransferChannel::free_buffers() noexcept
	{
		for(auto &buffer : buffers)
			parent_device->GetAllocator()->Release(buffer.bounded_buffer_size.bounded_buffer,
												   MemoryPoolOnEmptyPolicy::Free);
	}

	hrs::error TransferChannel::push_new_buffer(vk::DeviceSize common_size)
	{
		hrs::assert_true_debug(buffer_size_power != 0,
							   "New buffer size power must be greater than zero!");

		vk::DeviceSize rounded_common_size = hrs::round_up_size_to_alignment(common_size, rounding_size);
		vk::DeviceSize new_buffer_size = buffer_size_power * rounding_size;
		new_buffer_size = (new_buffer_size > rounded_common_size ? new_buffer_size : rounded_common_size);
		auto buffer_exp = allocate_buffer({new_buffer_size, buffer_alignment}, calc);
		if(!buffer_exp)
			return {buffer_exp.error()};

		buffers.push_back({std::move(buffer_exp.value()), new_buffer_size});
		return {};
	}
};
