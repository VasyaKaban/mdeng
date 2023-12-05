/**
 * @file
 *
 * Represents Transfer class for transfering data to device side
 */

#pragma once

#include "../Allocator/Allocator.hpp"
#include "../Allocator/ConditionalAllocate.hpp"
#include "../hrs/block.hpp"
#include "EmbeddedOperation.hpp"
#include "BufferRegion.hpp"
#include "ImageRegion.hpp"
#include <set>
#include <ranges>
#include <functional>
#include <variant>
#include <limits>

namespace FireLand
{
	template<std::ranges::range R, typename T>
	constexpr inline bool RangesSameValueType = std::is_same_v<std::ranges::range_value_t<R>, T>;

	template<std::ranges::range R, typename To>
	constexpr inline bool RangesConvertibleValueType = std::is_convertible_v<std::ranges::range_value_t<R>, To>;

	/**
	 * @brief The TransferObject using
	 *
	 * Used for dynamic dispatching transfer commands
	 */
	using TransferObject = std::variant<TransferBufferRegions, TransferImageRegions, EmbeddedOperation>;

	class Transfer
	{
	private:
		Transfer(Allocator *_allocator,
				 vk::CommandPool _command_pool,
				 vk::CommandBuffer _command_buffer,
				 vk::Fence _fence,
				 vk::Queue _queue,
				 uint32_t _queue_family_index);
	public:
		/**
		 * @brief Create
		 * @param _allocator allocator for future allocations
		 * @param _queue_family_index family queue index
		 * @param _queue_index queue index
		 * @return Transfer object or error result
		 */
		static hrs::expected<Transfer, vk::Result>
		Create(Allocator &_allocator,
			   uint32_t _queue_family_index,
			   uint32_t _queue_index);

		~Transfer();
		Transfer(const Transfer &) = delete;
		Transfer(Transfer &&transfer) noexcept;
		Transfer & operator=(const Transfer &) = delete;
		Transfer & operator=(Transfer &&transfer) noexcept;

		/**
		 * @brief operator bool
		 *
		 * Checks whether object is created or not
		 */
		constexpr explicit operator bool() const noexcept;

		/**
		 * @brief IsCreated
		 * @return true if object is created
		 *
		 * Same as operator bool
		 */
		constexpr bool IsCreated() const noexcept;

		/**
		 * @brief IsReadyForTransfer
		 * @return result according to vkGetFenceStatus
		 */
		vk::Result IsReadyForTransfer();

		/**
		 * @brief Wait
		 * @param timeout timeout value in nanoseconds to wait
		 * @return result value according to vkWaitForFences
		 */
		vk::Result Wait(uint64_t timeout = std::numeric_limits<uint64_t>::max());

		/**
		 * @brief TransferToBuffer
		 * @tparam R type that must satisfy sized_range concept
		 * @param regions range of buffer regions
		 * @param dst destination buffer
		 * @param data data pointer
		 * @param alignment the alignment value that must be used for placing data in transfer buffer
		 * @warning Data pointer must be alive until data is not transffered!
		 */
		template<std::ranges::sized_range R>
			requires RangesSameValueType<R, BufferRegion>
		void TransferToBuffer(const R &regions,
							  vk::Buffer dst,
							  std::byte *data,
							  vk::DeviceSize alignment);

		/**
		 * @brief TransferToImage
		 * @tparam R type that must satisfy sized_range concept
		 * @param regions range of image regions
		 * @param dst destination image
		 * @param texel_size image texel size
		 * @param layout image layout
		 * @param data data pointer
		 * @param alignment the alignment value that must be used for placing data in transfer buffer
		 * @warning Data pointer must be alive until data is not transffered!
		 */
		template<std::ranges::sized_range R>
			requires RangesSameValueType<R, ImageRegion>
		void TransferToImage(const R &regions,
							 vk::Image dst,
							 vk::DeviceSize texel_size,
							 vk::ImageLayout layout,
							 std::byte *data,
							 vk::DeviceSize alignment);


		/**
		 * @brief EmbedOperation
		 * @tparam R type that must satisfy sized_range concept
		 * @param operations range of embedded operations
		 */
		template<std::ranges::sized_range R>
			requires RangesConvertibleValueType<R, EmbeddedOperation>
		void EmbedOperation(const R &operations);

		/**
		 * @brief TransferToBuffer
		 * @param regions transfer buffer regions
		 * @param alignment the alignment value that must be used for placing data in transfer buffer
		 */
		void TransferToBuffer(TransferBufferRegions &&regions, vk::DeviceSize alignment);

		/**
		 * @brief TransferToImage
		 * @param regions transfer image regions
		 * @param alignment the alignment value that must be used for placing data in transfer buffer
		 */
		void TransferToImage(TransferImageRegions &&regions, vk::DeviceSize alignment);

		/**
		 * @brief TransferAnyObject
		 * @param object object for transferring
		 * @param alignment the alignment value that must be used for placing data in transfer buffer
		 */
		void TransferAnyObject(TransferObject &&object, vk::DeviceSize alignment);

		/**
		 * @brief Flush
		 * @tparam R type that must satisfy sized_range concept
		 * @param infos range of submit infos
		 * @return result of allocate and transfer operations
		 */
		template<std::ranges::sized_range R>
			requires RangesSameValueType<R, vk::SubmitInfo>
		vk::Result Flush(const R &infos);

		/**
		 * @brief GetBufferSize
		 * @return transfer buffer size
		 */
		constexpr vk::DeviceSize GetBufferSize() const noexcept;

		/**
		 * @brief GetBuffer
		 * @return transfer buffer object
		 */
		constexpr Buffer GetBuffer() const noexcept;

		/**
		 * @brief GetCommandBuffer
		 * @return command buffer object
		 */
		constexpr vk::CommandBuffer GetCommandBuffer() const noexcept;

	private:
		Allocator *allocator;///<pointer to allocator
		Buffer transfer_buffer;///<transfer buffer
		vk::CommandPool command_pool;///<command pool for command buffer
		vk::CommandBuffer command_buffer;///<command buffer
		vk::Fence commit_fence;///<fence for waiting on it before sending another commands
		vk::Queue commit_queue;///<queue for commiting commands
		uint32_t queue_family_index;///<queue family index for transfer buffer create info
		std::vector<TransferObject> objects;///<vector of objects to transfer
		vk::DeviceSize common_size;///<size that objects occupy at this time
	};

	inline Transfer::Transfer(Allocator *_allocator,
							  vk::CommandPool _command_pool,
							  vk::CommandBuffer _command_buffer,
							  vk::Fence _fence,
							  vk::Queue _queue,
							  uint32_t _queue_family_index)
	{
		allocator = _allocator;
		command_pool = _command_pool;
		command_buffer = _command_buffer;
		commit_fence = _fence;
		commit_queue = _queue;
		common_size = 0;
		queue_family_index = _queue_family_index;
	}

	/**
	 * @warning Aborts if allocator isn't created!
	 */
	inline hrs::expected<Transfer, vk::Result>
	Transfer::Create(Allocator &_allocator,
					 uint32_t _queue_family_index,
					 uint32_t _queue_index)
	{
		hrs::assert_true_debug(_allocator.IsCreated(), "Allocator isn't created yet!");
		vk::Device device = _allocator.GetDevice();
		vk::Queue queue = device.getQueue(_queue_family_index, _queue_index);
		/*auto destroy = [dev = alloc.GetDevice()](vk::CommandPool pool = {}, vk::CommandBuffer buffer = {}, vk::Fence fence = {})
		{
			if(fence)
				dev.destroy(fence);

			if(buffer)
				dev.free(pool, buffer);

			if(pool)
				dev.destroy(pool);
		};*/

		vk::CommandPoolCreateInfo pool_info(vk::CommandPoolCreateFlagBits::eTransient |
											vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
											_queue_family_index);

		vk::FenceCreateInfo fence_info(vk::FenceCreateFlagBits::eSignaled);
		auto pool = device.createCommandPoolUnique(pool_info);
		if(pool.result != vk::Result::eSuccess)
			return pool.result;

		vk::CommandBufferAllocateInfo command_buffer_info(pool.value.get(),
														  vk::CommandBufferLevel::ePrimary,
														  1);
		auto command_buffer = device.allocateCommandBuffersUnique(command_buffer_info);
		if(command_buffer.result != vk::Result::eSuccess)
			return command_buffer.result;

		auto fence = device.createFenceUnique(fence_info);
		if(fence.result != vk::Result::eSuccess)
			return fence.result;

		return Transfer(&_allocator,
						pool.value.release(),
						command_buffer.value[0].release(),
						fence.value.release(),
						queue,
						_queue_family_index);
	}

	/**
	 * @warning Aborts on release if wait result doesn't match vk::Result::eSuccess!
	 */
	inline Transfer::~Transfer()
	{
		if(*this)
		{
			vk::Result wait_result = Wait();
			hrs::assert_true(wait_result == vk::Result::eSuccess,
							 "Unexpected wait result -> {}",
							 vk::to_string(wait_result));

			vk::Device device = allocator->GetDevice();
			device.destroy(commit_fence);
			device.free(command_pool, command_buffer);
			device.destroy(command_pool);
			if(transfer_buffer)
				transfer_buffer.Destroy(device);
		}
	}

	/**
	 * @warning Aborts if Wait on this and wait on transfer don't match vk::Result::eSuccess!
	 */
	inline Transfer::Transfer(Transfer &&transfer) noexcept
	{
		hrs::assert_true_debug(Wait() == vk::Result::eSuccess, "Unexpected wait result!");
		hrs::assert_true_debug(transfer.Wait() == vk::Result::eSuccess, "Unexpected wait result!");
		allocator = transfer.allocator;
		transfer_buffer = std::move(transfer.transfer_buffer);
		command_pool = std::move(transfer.command_pool);
		command_buffer = std::move(transfer.command_buffer);
		commit_fence = std::move(transfer.commit_fence);
		commit_queue = std::move(transfer.commit_queue);
		objects = std::move(transfer.objects);
		common_size = transfer.common_size;
	}

	/**
	 * @warning Aborts if Wait on this donesn't match vk::Result::eSuccess!
	 */
	inline auto Transfer::operator=(Transfer &&transfer) noexcept -> Transfer &
	{
		vk::Result wait_result = Wait();
		hrs::assert_true(wait_result == vk::Result::eSuccess,
						 "Unexpected wait result -> {}",
						 vk::to_string(wait_result));

		this->~Transfer();
		allocator = transfer.allocator;
		transfer_buffer = std::move(transfer.transfer_buffer);
		command_pool = std::move(transfer.command_pool);
		command_buffer = std::move(transfer.command_buffer);
		commit_fence = std::move(transfer.commit_fence);
		commit_queue = std::move(transfer.commit_queue);
		objects = std::move(transfer.objects);
		common_size = transfer.common_size;
		return *this;
	}

	constexpr Transfer::operator bool() const noexcept
	{
		return allocator &&
			   transfer_buffer &&
			   command_pool &&
			   command_buffer &&
			   commit_fence &&
			   commit_queue;
	}

	constexpr bool Transfer::IsCreated() const noexcept
	{
		return static_cast<bool>(*this);
	}

	inline vk::Result Transfer::IsReadyForTransfer()
	{
		if(!IsCreated())
			return vk::Result::eSuccess;

		return allocator->GetDevice().getFenceStatus(commit_fence);
	}

	inline vk::Result Transfer::Wait(uint64_t timeout)
	{
		if(!IsCreated())
			return vk::Result::eSuccess;

		return allocator->GetDevice().waitForFences(commit_fence, VK_TRUE, timeout);
	}

	/**
	 * @warning Aborts if alignment is not power of two
	 */
	template<std::ranges::sized_range R>
		requires RangesSameValueType<R, BufferRegion>
	void Transfer::TransferToBuffer(const R &regions,
									vk::Buffer dst,
									std::byte *data,
									vk::DeviceSize alignment)
	{
		hrs::assert_true_debug(hrs::is_power_of_two(alignment), "Alignment isn't power of two!");
		if(std::ranges::size(regions) == 0)
			return;

		//if(!hrs::is_multiple_of(common_size, alignment))
		//	common_size = hrs::round_up_size_to_alignment(common_size, alignment);

		std::vector<vk::BufferCopy> copy_regions;
		std::vector<std::size_t> regions_offsets;
		copy_regions.reserve(std::ranges::size(regions));
		regions_offsets.reserve(std::ranges::size(regions));
		for(const auto &reg : regions)
		{
			common_size = hrs::round_up_size_to_alignment(common_size, alignment);
			//hrs::assert_true_debug(hrs::is_multiple_of(reg.copy.size, alignment),
			//				 "Region size doesn't satisfy alignment!");
			copy_regions.push_back(reg.copy);
			copy_regions.back().srcOffset = common_size;
			regions_offsets.push_back(reg.data_offset);
			common_size += copy_regions.back().size;
		}

		objects.push_back(TransferBufferRegions(dst, std::move(copy_regions), std::move(regions_offsets), data));
	}

	/**
	 * @warning Aborts if alignment is not power of two
	 */
	template<std::ranges::sized_range R>
		requires RangesSameValueType<R, ImageRegion>
	void Transfer::TransferToImage(const R &regions,
								   vk::Image dst,
								   vk::DeviceSize texel_size,
								   vk::ImageLayout layout,
								   std::byte *data,
								   vk::DeviceSize alignment)
	{
		if(texel_size == 0)
			return;

		hrs::assert_true_debug(hrs::is_power_of_two(alignment), "Alignment isn't power of two!");
		if(std::ranges::size(regions) == 0)
			return;

		//if(!hrs::is_multiple_of(common_size, alignment))
		//	common_size = hrs::round_up_size_to_alignment(common_size, alignment);

		std::vector<vk::BufferImageCopy> copy_regions;
		std::vector<std::size_t> regions_offsets;
		copy_regions.reserve(std::ranges::size(regions));
		for(const auto &reg : regions)
		{
			common_size = hrs::round_up_size_to_alignment(common_size, alignment);
			//hrs::assert_true_debug(hrs::is_multiple_of(reg.size, alignment),
			//				 "Region size doesn't satisfy alignment!");
			copy_regions.push_back(reg.copy);
			copy_regions.back().bufferOffset = common_size;
			regions_offsets.push_back(reg.data_offset);
			common_size += TransferImageRegions::GetSize(copy_regions.back(), texel_size);
		}

		objects.push_back(TransferImageRegions(dst,
											   layout,
											   texel_size,
											   std::move(copy_regions),
											   std::move(regions_offsets),
											   data));
	}

	template<std::ranges::sized_range R>
		requires RangesConvertibleValueType<R, EmbeddedOperation>
	void Transfer::EmbedOperation(const R &operations)
	{	
		std::size_t operations_size = std::ranges::size(operations);
		if(operations_size == 0)
			return;

		if(objects.capacity() - objects.size() < operations_size)
			objects.reserve((operations_size - (objects.capacity() - objects.size())) + objects.capacity());

		for(auto &op : operations)
			objects.push_back(EmbeddedOperation(op));
	}

	/**
	 * @warning Aborts if alignment is not power of two and
	 * if regions and offsets sizes aren't equal to each other!
	 */
	inline void Transfer::TransferToBuffer(TransferBufferRegions &&regions, vk::DeviceSize alignment)
	{
		hrs::assert_true_debug(hrs::is_power_of_two(alignment), "Alignment isn't power of two!");
		std::visit([alignment = alignment, this, &regions]<typename BT>(BT &value)
		{
			if constexpr(std::same_as<BT, RegionOffset<vk::BufferCopy>>)
			{
				RegionOffset<vk::BufferCopy> &reg = value;
				common_size = hrs::round_up_size_to_alignment(common_size, alignment);
				reg.region.srcOffset = common_size;
				common_size += reg.region.size;
			}
			else
			{
				RegionsOffsets<vk::BufferCopy> &regs = value;
				if(regs.regions.empty())
					return;

				hrs::assert_true_debug(regs.regions.size() == regs.regions_offsets.size(),
									   "Regions and offsets count must be equal!");

				for(std::size_t i = 0; i < regs.regions.size(); i++)
				{
					common_size = hrs::round_up_size_to_alignment(common_size, alignment);
					regs.regions[i].srcOffset = common_size;
					common_size += regs.regions[i].size;
				}
			}
			objects.push_back(std::move(regions));
		}, regions.regions);
	}

	/**
	 * @warning Aborts if alignment is not power of two and
	 * if regions and offsets sizes aren't equal to each other!
	 */
	inline void Transfer::TransferToImage(TransferImageRegions &&regions, vk::DeviceSize alignment)
	{
		hrs::assert_true_debug(hrs::is_power_of_two(alignment), "Alignment isn't power of two!");
		std::visit([alignment = alignment, this, &regions]<typename BT>(BT &value)
		{
			if constexpr(std::same_as<BT, RegionOffset<vk::BufferImageCopy>>)
			{
				RegionOffset<vk::BufferImageCopy> &reg = value;
				common_size = hrs::round_up_size_to_alignment(common_size, alignment);
				reg.region.bufferOffset = common_size;
				common_size += regions.GetSize(0);
			}
			else
			{
				RegionsOffsets<vk::BufferImageCopy> &regs = value;
				if(regs.regions.empty())
					return;

				hrs::assert_true_debug(regs.regions.size() == regs.regions_offsets.size(),
									   "Regions and offsets count must be equal!");

				for(std::size_t i = 0; i < regs.regions.size(); i++)
				{
					common_size = hrs::round_up_size_to_alignment(common_size, alignment);
					regs.regions[i].bufferOffset = common_size;
					common_size += regions.GetSize(i);
				}
			}

			objects.push_back(std::move(regions));
		}, regions.regions);
	}

	inline void Transfer::TransferAnyObject(TransferObject &&object, vk::DeviceSize alignment)
	{
		std::visit([alignment = alignment, this]<typename T>(T &obj)
		{
			if constexpr(std::same_as<T, TransferBufferRegions>)
				TransferToBuffer(std::move(obj), alignment);
			else if constexpr(std::same_as<T, TransferImageRegions>)
				TransferToImage(std::move(obj), alignment);
			else if constexpr(std::same_as<T, EmbeddedOperation>)
				EmbedOperation(std::ranges::single_view{std::move(obj)});
		}, object);
	}

	template<std::ranges::sized_range R>
		requires RangesSameValueType<R, vk::SubmitInfo>
	vk::Result Transfer::Flush(const R &infos)
	{
		if(objects.empty())
			return vk::Result::eSuccess;

		if(vk::Result wait_res = Wait(); wait_res != vk::Result::eSuccess)
			return wait_res;

		//resize
		if(transfer_buffer && transfer_buffer.memory.size < common_size)
			transfer_buffer.Destroy(allocator->GetDevice());

		if(!transfer_buffer)
		{
			vk::BufferCreateInfo buffer_info({},
											 common_size,
											 vk::BufferUsageFlagBits::eTransferSrc,
											 vk::SharingMode::eExclusive,
											 1,
											 &queue_family_index);

			constexpr static std::array transfer_conds
			{
				AllocateConditionalInfo{DesiredType::Only, vk::MemoryPropertyFlagBits::eHostVisible, true},
				AllocateConditionalInfo{DesiredType::Any, vk::MemoryPropertyFlagBits::eHostVisible, true},
			};

			auto alloc_result = AllocateBuffer(*allocator, transfer_conds, buffer_info);
			if(!alloc_result.has_value())
				return vk::Result::eErrorOutOfHostMemory;

			transfer_buffer = std::move(alloc_result.value().first);
		}

		//write commands
		vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		auto begin_res = command_buffer.begin(begin_info);
		if(begin_res != vk::Result::eSuccess)
			return begin_res;

		for(auto &obj : objects)
		{
			std::visit([&]<typename T>(const T &value)
			{
				if constexpr(std::same_as<T, TransferBufferRegions>)
				{
					const TransferBufferRegions &buffer_regions = value;
					buffer_regions.Transfer(transfer_buffer.buffer, transfer_buffer.memory.map_ptr, command_buffer);
				}
				else if constexpr(std::same_as<T, TransferImageRegions>)
				{
					const TransferImageRegions &image_regions = value;
					image_regions.Transfer(transfer_buffer.buffer, transfer_buffer.memory.map_ptr, command_buffer);
				}
				else if constexpr(std::same_as<T, EmbeddedOperation>)
				{
					const EmbeddedOperation &op = value;
					if(op)
						op(command_buffer);
				}
			}, obj);
		}

		objects.clear();
		common_size = 0;
		if(auto reset_result = allocator->GetDevice().resetFences(commit_fence);
			reset_result != vk::Result::eSuccess)
		{
			command_buffer.reset();
			return reset_result;
		}

		auto end_result = command_buffer.end();
		if(end_result != vk::Result::eSuccess)
			return end_result;

		vk::SubmitInfo main_info(0,
								 nullptr,
								 nullptr,
								 1,
								 &command_buffer,
								 0,
								 nullptr);
		std::vector<vk::SubmitInfo> submits;
		submits.reserve(std::ranges::size(infos) + 1);
		submits.insert(submits.end(), std::ranges::begin(infos), std::ranges::end(infos));
		submits.push_back(main_info);

		vk::Result res = commit_queue.submit(submits, commit_fence);
		if(res != vk::Result::eSuccess)
		{
			command_buffer.reset();
			return res;
		}

		return vk::Result::eSuccess;
	}

	constexpr vk::DeviceSize Transfer::GetBufferSize() const noexcept
	{
		return transfer_buffer.memory.size;
	}

	constexpr Buffer Transfer::GetBuffer() const noexcept
	{
		return transfer_buffer;
	}

	constexpr vk::CommandBuffer Transfer::GetCommandBuffer() const noexcept
	{
		return command_buffer;
	}
};
