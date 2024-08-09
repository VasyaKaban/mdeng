#include "TransferChannel.h"
#include "../Allocator/Allocator.h"
#include "../Context/DeviceLoader.h"
#include "../Vulkan/codegen/loader_check_begin.h"
#include "../Vulkan/VkResultMeta.hpp"
#include <execution>

namespace FireLand
{
	TransferChannel::TransferChannel(VkDevice _device,
									 const DeviceLoader *_dl,
									 Allocator *_allocator,
									 VkFence _wait_fence,
									 const QueueFamilyIndex &_transfer_queue,
									 VkDeviceSize _buffer_rounding_size,
									 VkCommandBuffer _command_buffer,
									 const VkAllocationCallbacks *_allocation_callbacks) noexcept
		: device(_device),
		  dl(_dl),
		  allocator(_allocator),
		  wait_fence(_wait_fence),
		  transfer_queue(_transfer_queue),
		  write_state(TransferChannelWriteState::Flushed),
		  buffer_rounding_size(_buffer_rounding_size),
		  command_buffer(_command_buffer),
		  allocation_callbacks(_allocation_callbacks)
	{
		hrs::assert_true_debug(buffer_rounding_size != 0,
							   "Buffer rounding size must be greater than zero!");
	}

	TransferChannel::TransferChannel() noexcept
		: device(VK_NULL_HANDLE) {}

	TransferChannel::~TransferChannel()
	{
		Destroy();
	}

	TransferChannel::TransferChannel(TransferChannel &&tc) noexcept
		: device(std::exchange(tc.device, VK_NULL_HANDLE)),
		  dl(tc.dl),
		  allocator(tc.allocator),
		  buffers(std::move(tc.buffers)),
		  wait_fence(tc.wait_fence),
		  transfer_queue(tc.transfer_queue),
		  write_state(tc.write_state),
		  buffer_rounding_size(tc.buffer_rounding_size),
		  command_buffer(tc.command_buffer),
		  allocation_callbacks(tc.allocation_callbacks) {}

	TransferChannel & TransferChannel::operator=(TransferChannel &&tc) noexcept
	{
		Destroy();

		device = std::exchange(tc.device, VK_NULL_HANDLE);
		dl = tc.dl;
		allocator = tc.allocator;
		buffers = std::move(tc.buffers);
		wait_fence = tc.wait_fence;
		transfer_queue = tc.transfer_queue;
		write_state = tc.write_state;
		buffer_rounding_size = tc.buffer_rounding_size;
		command_buffer = tc.command_buffer;
		allocation_callbacks = tc.allocation_callbacks;

		return *this;
	}

	hrs::expected<TransferChannel, InitResult>
	TransferChannel::Create(VkDevice _device,
							const DeviceLoader &_dl,
							Allocator &_allocator,
							const QueueFamilyIndex &_transfer_queue,
							VkCommandBuffer _command_buffer,
							VkDeviceSize _buffer_rounding_size,
							const VkAllocationCallbacks *_allocation_callbacks)
	{
		hrs::assert_true_debug(_device != VK_NULL_HANDLE, "Device isn't created yet!");
		hrs::assert_true_debug(_allocator.IsCreated(), "Allocator isn't created yet!");
		hrs::assert_true_debug(_transfer_queue.queue != VK_NULL_HANDLE, "Queue isn't retrieved yet!");
		hrs::assert_true_debug(_command_buffer != VK_NULL_HANDLE, "Command buffer isn't created yet!");
		hrs::assert_true_debug(_buffer_rounding_size > 0, "Buffer rounding size must be greater than zero!");

		FIRE_LAND_LOADER_CHECK_USE(_dl)
			FIRE_LAND_LOADER_CHECK_FUNCTION(vkCreateFence)
			FIRE_LAND_LOADER_CHECK_FUNCTION(vkDestroyFence)
			FIRE_LAND_LOADER_CHECK_FUNCTION(vkWaitForFences)
			FIRE_LAND_LOADER_CHECK_FUNCTION(vkGetFenceStatus)
			FIRE_LAND_LOADER_CHECK_FUNCTION(vkResetFences)
			FIRE_LAND_LOADER_CHECK_FUNCTION(vkBeginCommandBuffer)
			FIRE_LAND_LOADER_CHECK_FUNCTION(vkEndCommandBuffer)
			FIRE_LAND_LOADER_CHECK_FUNCTION(vkCmdPipelineBarrier)
			FIRE_LAND_LOADER_CHECK_FUNCTION(vkCmdCopyBuffer)
			FIRE_LAND_LOADER_CHECK_FUNCTION(vkCmdCopyBufferToImage)
			FIRE_LAND_LOADER_CHECK_FUNCTION(vkQueueSubmit)
		FIRE_LAND_LOADER_CHECK_UNUSE()

		constexpr static VkFenceCreateInfo fence_info =
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		VkFence _wait_fence;
		VkResult res = _dl.vkCreateFence(_device, &fence_info, _allocation_callbacks, &_wait_fence);
		if(res != VK_SUCCESS)
			return res;

		return TransferChannel(_device,
							   &_dl,
							   &_allocator,
							   _wait_fence,
							   _transfer_queue,
							   _buffer_rounding_size,
							   _command_buffer,
							   _allocation_callbacks);
	}

	void TransferChannel::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		VkResult wait_res = WaitFence();
		hrs::assert_true(wait_res == VK_SUCCESS, "Bad WaitFence result = {}!",
						 hrs::enum_meta<VkResult>::get_name(wait_res));

		dl->vkDestroyFence(device, wait_fence, allocation_callbacks);
		for(const auto &buffer : buffers)
			allocator->Free(buffer, MemoryPoolOnEmptyPolicy::Free);

		buffers.clear();
		device = VK_NULL_HANDLE;
	}

	bool TransferChannel::IsCreated() const noexcept
	{
		return device != VK_NULL_HANDLE;
	}

	VkResult TransferChannel::Flush(std::span<const VkSubmitInfo> submits) noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Transfer channel isn't created yet!");
		hrs::assert_true_debug(write_state == TransferChannelWriteState::WriteEnded,
							   "Write state must be 'Ended'!");

		VkResult res = dl->vkQueueSubmit(transfer_queue.queue, submits.size(), submits.data(), wait_fence);
		if(res == VK_SUCCESS)
			write_state = TransferChannelWriteState::Flushed;

		return res;
	}

	VkResult TransferChannel::GetWaitFenceStatus() const noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Transfer channel isn't created yet!");
		return dl->vkGetFenceStatus(device, wait_fence);
	}

	VkResult TransferChannel::WaitFence(std::uint64_t timeout) noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Transfer channel isn't created yet!");
		return dl->vkWaitForFences(device, 1, &wait_fence, VK_TRUE, timeout);
	}

	VkDevice TransferChannel::GetDevice() const noexcept
	{
		return device;
	}

	const DeviceLoader * TransferChannel::GetDeviceLoader() const noexcept
	{
		return dl;
	}

	Allocator * TransferChannel::GetAllocator() noexcept
	{
		return allocator;
	}

	const Allocator * TransferChannel::GetAllocator() const noexcept
	{
		return allocator;
	}

	const QueueFamilyIndex & TransferChannel::GetTransferQueue() const noexcept
	{
		return transfer_queue;
	}

	VkDeviceSize TransferChannel::GetBufferRoundingSize() const noexcept
	{
		return buffer_rounding_size;
	}

	VkCommandBuffer TransferChannel::GetCommandBuffer() const noexcept
	{
		return command_buffer;
	}

	const std::vector<BoundedBufferSizeFillness> & TransferChannel::GetBuffers() const noexcept
	{
		return buffers;
	}

	VkFence TransferChannel::GetWaitFence() const noexcept
	{
		return wait_fence;
	}

	VkResult TransferChannel::Begin(const VkCommandBufferBeginInfo &info) noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Transfer channel isn't created yet!");
		hrs::assert_true_debug(write_state != TransferChannelWriteState::WriteEnded,
							   "Write state cannot be 'WriteEnded'!");
		if(write_state == TransferChannelWriteState::WriteStarted)
			return VK_SUCCESS;

		//wait fence
		//clear buffers
		VkResult res = WaitFence();
		if(res != VK_SUCCESS)
			return res;

		for(auto &buffer : buffers)
			buffer.fillness = 0;

		res = dl->vkBeginCommandBuffer(command_buffer, &info);
		if(res == VK_SUCCESS)
			write_state = TransferChannelWriteState::WriteStarted;

		return res;
	}

	VkResult TransferChannel::End() noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Transfer channel isn't created yet!");
		hrs::assert_true_debug(write_state != TransferChannelWriteState::Flushed,
							   "Write state cannot be 'Flushed'!");
		if(write_state == TransferChannelWriteState::WriteEnded)
			return VK_SUCCESS;

		VkResult res = dl->vkEndCommandBuffer(command_buffer);
		if(res != VK_SUCCESS)
			return res;

		res = dl->vkResetFences(device, 1, &wait_fence);
		if(res != VK_SUCCESS)
			return res;

		write_state = TransferChannelWriteState::WriteEnded;

		return res;
	}

	TransferChannelWriteState TransferChannel::GetWriteState() const noexcept
	{
		return write_state;
	}

	hrs::error
	TransferChannel::CopyBuffer(VkBuffer dst_buffer,
								std::span<const std::byte *> datas,
								std::span<const TransferBufferOpRegion> regions)
	{
		hrs::assert_true_debug(IsCreated(), "Transfer channel isn't created yet!");
		hrs::assert_true_debug(dst_buffer != VK_NULL_HANDLE, "Destination buffer isn't created yet!");
		hrs::assert_true(write_state == TransferChannelWriteState::WriteStarted,
						 "Writing has not been started yet!");

		VkDeviceSize common_size = 0;
		for(const auto &region : regions)
		{
			//just use 4-byte alignment
			common_size += hrs::round_up_size_to_alignment(region.data_blk.size, 4);
		}

		for(auto &buffer : buffers)
		{
			if(auto opt = buffer.Append({common_size, 4}); opt)
			{
				copy_buffer(dst_buffer, buffer, datas, regions, *opt);
				return {};
			}
		}

		//allocate
		auto err = InsertBuffer(common_size);
		if(err)
			return err;


		auto opt = buffers.back().Append({common_size, 4});
		hrs::assert_true_debug(opt.has_value(), "Contract violation! Append must return valid offset!");

		copy_buffer(dst_buffer, buffers.back(), datas, regions, *opt);
		return {};

	}

	hrs::error
	TransferChannel::CopyImageSubresource(VkImage dst_image,
										  VkImageLayout image_layout,
										  VkDeviceSize block_size,
										  std::span<const std::byte *> datas,
										  std::span<const TransferImageOpRegion> regions)
	{
		//DO NOT FORGET ABOUT 4-bytes alignment!!!
		hrs::assert_true_debug(IsCreated(), "Transfer channel isn't created yet!");
		hrs::assert_true_debug(dst_image != VK_NULL_HANDLE, "Destination image isn't created yet!");
		hrs::assert_true(block_size != 0, "The block size must be greater than zero!");
		hrs::assert_true(write_state == TransferChannelWriteState::WriteStarted,
						 "Writing has not been started yet!");

		VkDeviceSize common_alignment = block_size * 4;//multiple of 4 and block_size
		VkDeviceSize common_size = 0;
		for(const auto &region : regions)
			common_size += hrs::round_up_size_to_alignment(image_data_size(region.image_extent, block_size),
														   common_alignment);

		for(auto &buffer : buffers)
		{
			if(auto opt = buffer.Append({common_size, 4}); opt)
			{
				copy_image(dst_image, image_layout, block_size, buffer, datas, regions, *opt);
				return {};
			}
		}

		//allocate
		auto err = InsertBuffer(common_size);
		if(err)
			return err;


		auto opt = buffers.back().Append({common_size, common_alignment});
		hrs::assert_true_debug(opt.has_value(), "Contract violation! Append must return valid offset!");

		copy_image(dst_image, image_layout, block_size, buffers.back(), datas, regions, *opt);
		return {};
	}

	hrs::expected<EmbedResult, hrs::error> TransferChannel::Embed(const hrs::mem_req<VkDeviceSize> &req)
	{
		hrs::assert_true_debug(IsCreated(), "Transfer channel isn't created yet!");
		hrs::assert_true(write_state == TransferChannelWriteState::WriteStarted,
						 "Writing has not been started yet!");

		for(auto &buffer : buffers)
		{
			auto append_opt = buffer.Append(req);
			if(append_opt)
				return
					EmbedResult
					{
						.command_buffer = command_buffer,
						.buffer = buffer.buffer,
						.offset = *append_opt
					};
		}

		hrs::error err = InsertBuffer(req.size);
		if(err)
			return err;

		auto opt = buffers.back().Append(req);
		hrs::assert_true_debug(opt.has_value(), "Contract violation! Append must return valid offset!");

		return
			EmbedResult
			{
				.command_buffer = command_buffer,
				.buffer = buffers.back().buffer,
				.offset = *opt
			};
	}

	void TransferChannel::EmbedBarrier(VkPipelineStageFlags src_stages,
									   VkPipelineStageFlags dst_stages,
									   VkDependencyFlags dependency,
									   std::span<const VkMemoryBarrier> memory_barriers,
									   std::span<const VkBufferMemoryBarrier> buffer_memory_barriers,
									   std::span<const VkImageMemoryBarrier> image_memory_barriers) noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Transfer channel isn't created yet!");
		hrs::assert_true(write_state == TransferChannelWriteState::WriteStarted,
						 "Writing has not been started yet!");
		dl->vkCmdPipelineBarrier(command_buffer,
								 src_stages,
								 dst_stages,
								 dependency,
								 memory_barriers.size(),
								 memory_barriers.data(),
								 buffer_memory_barriers.size(),
								 buffer_memory_barriers.data(),
								 image_memory_barriers.size(),
								 image_memory_barriers.data());
	}

	hrs::error TransferChannel::FlattenBuffers()
	{
		hrs::assert_true_debug(IsCreated(), "Transfer channel isn't created yet!");
		hrs::assert_true(write_state == TransferChannelWriteState::Flushed,
						 "Write state must be 'Flushed'!");

		VkResult res = WaitFence();
		if(res != VK_SUCCESS)
			return res;

		VkDeviceSize common_size = 0;
		for(auto &buffer : buffers)
		{
			common_size += buffer.size;
			allocator->Free(buffer, MemoryPoolOnEmptyPolicy::Free);
		}

		buffers.clear();
		if(common_size == 0)
			return {};

		return InsertBuffer(common_size);
	}

	hrs::error TransferChannel::InsertBuffer(VkDeviceSize size)
	{
		hrs::assert_true_debug(IsCreated(), "Transfer channel isn't created yet!");
		if(!hrs::is_multiple_of(size, buffer_rounding_size))
			size = hrs::round_up_size_to_alignment(size, buffer_rounding_size);

		return allocate_insert_buffer(size);
	}

	hrs::error TransferChannel::allocate_insert_buffer(VkDeviceSize size)
	{
		const VkBufferCreateInfo buffer_info =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = {},
			.size = size,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = &transfer_queue.family_index
		};

		VkMemoryPropertyFlags memory_property;
		MemoryTypeSatisfyOp op;
		hrs::flags<AllocationFlags> flags;

		constexpr static std::array desired =
		{
			//only host visible(without coherent)
			MultipleAllocateDesiredOptions
			{
				.memory_property = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				.op = MemoryTypeSatisfyOp::Only,
				.flags = AllocationFlags::MapMemory
			},
			//host visible with any other properties
			MultipleAllocateDesiredOptions
			{
				.memory_property = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				.op = MemoryTypeSatisfyOp::Any,
				.flags = AllocationFlags::MapMemory
			},
			//allow place with mixed
			MultipleAllocateDesiredOptions
			{
				.memory_property = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				.op = MemoryTypeSatisfyOp::Any,
				.flags = hrs::flags(AllocationFlags::MapMemory) |
						 AllocationFlags::AllowPlaceWithMixedResources
			},
		};

		auto buffer_exp = allocator->Allocate(buffer_info, desired);
		if(!buffer_exp)
			return buffer_exp.error();

		buffers.push_back(BoundedBufferSizeFillness({std::move(buffer_exp->first), size}, 0));
		return {};
	}

	void TransferChannel::copy_buffer(VkBuffer dst_buffer,
									  BoundedBufferSizeFillness &src_buffer,
									  std::span<const std::byte *> datas,
									  std::span<const TransferBufferOpRegion> regions,
									  VkDeviceSize offset) noexcept
	{
		std::vector<VkBufferCopy> copies;
		copies.reserve(regions.size());
		for(const auto &region : regions)
		{
			hrs::assert_true_debug(datas.size() > region.data_index,
								   "Region data index = {} is out of bound = {}!",
								   region.data_index,
								   datas.size());

			const VkBufferCopy copy =
			{
				.srcOffset = offset,
				.dstOffset = region.dst_buffer_offset,
				.size = region.data_blk.size
			};

			copies.push_back(copy);

			std::copy_n(std::execution::unseq,
						datas[region.data_index] + region.data_blk.offset,
						region.data_blk.size,
						src_buffer.GetBufferMapPtr());

			offset += hrs::round_up_size_to_alignment(region.data_blk.size, 4);
		}

		dl->vkCmdCopyBuffer(command_buffer,
							src_buffer.buffer,
							dst_buffer,
							copies.size(),
							copies.data());
	}

	void TransferChannel::copy_image(VkImage dst_image,
									 VkImageLayout image_layout,
									 VkDeviceSize block_size,
									 BoundedBufferSizeFillness &src_buffer,
									 std::span<const std::byte *> datas,
									 std::span<const TransferImageOpRegion> regions,
									 VkDeviceSize offset) noexcept
	{
		std::vector<VkBufferImageCopy> copies;
		copies.reserve(regions.size());
		for(const auto &region : regions)
		{
			hrs::assert_true_debug(datas.size() > region.data_index,
								   "Region data index = {} is out of bound = {}!",
								   region.data_index,
								   datas.size());

			const VkBufferImageCopy copy =
			{
				.bufferOffset = offset,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = region.subresource_layers,
				.imageOffset = {0, 0, 0},
				.imageExtent = region.image_extent
			};

			copies.push_back(copy);

			VkDeviceSize region_size = image_data_size(region.image_extent, block_size);

			std::copy_n(std::execution::unseq,
						datas[region.data_index] + region.data_offset,
						region_size,
						src_buffer.GetBufferMapPtr());

			offset += hrs::round_up_size_to_alignment(region_size, block_size * 4);
		}

		dl->vkCmdCopyBufferToImage(command_buffer,
								   src_buffer.buffer,
								   dst_image,
								   image_layout,
								   copies.size(),
								   copies.data());
	}

	VkDeviceSize TransferChannel::image_data_size(const VkExtent3D &extent,
												  VkDeviceSize block_size) const noexcept
	{
		return extent.width * extent.height * extent.depth * block_size;
	}
};
