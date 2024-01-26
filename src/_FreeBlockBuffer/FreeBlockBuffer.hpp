/**
 * @file
 *
 * Represents FreeBlockBuffer class
 */
#pragma once

/*
Delete -> D, update -> U, add(no alloc) -> AN, add(device alloc) -> AD, add(host alloc) -> AH
0. *Nothing -> noop.
1. D -> same as 0.
2. U ->
	1. Transfer updates to buffer
	2. return 1 command.
3. AN -> same as 2.
4. AD ->
	1. allocate new device memory
	2. Embedded operation: copy from device_old to device_new
	3. Transfer from added to buffer
	3. swap device_old and device_new
	4. return 2, 3 commands.
5. AH ->
	1. Allocate host memory
	2. Embedded operation: copy from device to host memory
	3. transfer added to host memory
	4. Swap device memory and host memory
	5. return 2, 3, 4 commands.

6. D, U -> same as 2.
7. D, AN -> same as 2.
8. D, AD -> same as 4.
9. D, AH -> same as 5.

10. U, AN -> same as 2.
11. U, AD -> same as 4(+3 -> transfer from updates and added to buffer).
12. U, AH -> same as 5.

13. D, U, AN -> same as 2.
14. D, U, AD -> same as 4(+3 -> transfer from updates and added to buffer).
15. D, U, AH -> same as 5(+4 -> transfer from updates and added to host buffer).

+lookup for device allocation in future(if needed)
if(allocation is !device_local)
{
	1. allocate device memory
	2. Embedded operation: copy from host memory to device memory
	4. transfer data from host memory to device memory(if needed).
	4. Swap host memory and device memory
}

Maybe make only one request for update where we send all data to structure?
ex.
We send only one structure that has delete, update and add regions.
Firstly we delete, then add and check target state, and finnaly update and return commands

AH:
1. Allocate host memory
2. Embedded operation: copy from device to host memory
3. Embedded operation: barrier for host memory
4. transfer data to host memory
5. Swap device memory and host memory

D, AH -> same as above.

D, U, AH
1. Allocate host memory
2. Embedded operation: copy from device to host memory
3. Embedded operation: barrier for host memory
4. transfer data(U and AH) to host memory
5. Swap device memory and host memory

+lookup for device allocation in future
if(allocation is !device_local)
{
	1. allocate device memory
	2. Embedded operation: copy from host memory to device memory
	3. Swap host memory and device memory
}
 */

#include "../hrs/free_block_allocator.hpp"
#include "../Transfer/Transfer.hpp"
#include "../VulkanUtils/BufferWithProperty.hpp"

namespace FireLand
{
	/**
	 * @brief The UpdateRegion class
	 *
	 * Introduces region for update operation
	 */
	struct UpdateRegion
	{
		std::byte *data;///<pointer to data
		hrs::block<vk::DeviceSize> update_region;///<region which is needed to be updated
		vk::DeviceSize data_offset;///<offset within data pointer
	};

	/**
	 * @brief The AddRegion class
	 *
	 * Introduces data add operation
	 */
	struct AddRegion
	{
		std::byte *data;///<pointer to data
		vk::DeviceSize data_offset;///<offset within data pointer
		vk::DeviceSize data_size;///<size of data to add
	};

	/**
	 * @brief The DUAregions class
	 *
	 * Compound structure that includes delete, update and add regions
	 */
	struct DUAregions
	{
		std::vector<hrs::block<vk::DeviceSize>> delete_regions;///<regions to delete
		std::vector<UpdateRegion> update_regions;///<regions to update
		std::vector<AddRegion> add_regions;///<regions to add
	};

	/**
	 * @brief The Commands class
	 *
	 * Structure that contains array of added blocks for add operations and commands
	 * that must be executed to make data transfer
	 */
	struct Commands
	{
		/**
		 * @brief The BufferState enum
		 *
		 * Carries new buffer state after operations
		 */
		enum BufferState
		{
			None,///<no event(no allocations)
			NewDeviceAllocation,///<buffer was reallocated in device memory
			NewHostAllocation///<buffer was reallocated in host memory
		} state;///<buffer state

		std::vector<hrs::block<vk::DeviceSize>> added_blocks;///<blocks for add operations
		std::vector<TransferObject> commands_chain;///<transfer commands
	};

	/**
	 * @brief The FreeBlockBuffer class
	 *
	 * FreeBlockBuffer is a buffer that control it's data through free block buffer.
	 * The data is this buffer can be removed and added in any order but it causes a data fragmentation!
	 * Also this buffer became an owner of two buffers: target and vault.
	 * Target buffer is a buffer that is in use at this time.
	 * Vault buffer is a buffer that was a target buffer but it has been swapped and still carries
	 * data that aren't transferred yet. This buffer will be freed after next ReceiveCommands call.
	 */
	class FreeBlockBuffer
	{
	private:
		/**
		 * @brief FreeBlockBuffer
		 * @param _allocator allocator that will be used for memory allocations
		 * @param _buffer allocated target buffer
		 * @param _blocks free_block_allocator for this buffer
		 * @param _usage buffer usage for buffer create info
		 * @param _queue_family_index queue family index for bffer create info
		 */
		FreeBlockBuffer(Allocator *_allocator,
						BufferWithProperty &&_buffer,
						hrs::free_block_allocator<vk::DeviceSize> &&_blocks,
						vk::BufferUsageFlags _usage,
						uint32_t _queue_family_index);
	public:
		/**
		 * @brief Create
		 * @param _allocator allocator that will be used for memory allocations
		 * @param _alignnment buffer block alignment(same as block size)
		 * @param _blocks_count count of blocks that buffer will have at startup
		 * @param _usage buffer usage for buffer create info
		 * @param _queue_family_index queue family index for bffer create info
		 * @return Created FreeBlockBuffer or result error
		 */
		static hrs::expected<FreeBlockBuffer, vk::Result>
		Create(Allocator *_allocator,
			   vk::DeviceSize _alignnment,
			   vk::DeviceSize _blocks_count,
			   vk::BufferUsageFlags _usage,
			   uint32_t _queue_family_index);

		~FreeBlockBuffer();
		FreeBlockBuffer(const FreeBlockBuffer &) = delete;
		FreeBlockBuffer(FreeBlockBuffer &&buf) noexcept;
		FreeBlockBuffer & operator=(const FreeBlockBuffer &) = delete;
		FreeBlockBuffer & operator=(FreeBlockBuffer &&buf) noexcept;

		/**
		 * @brief GetTargetMemoryProperty
		 * @return target buffer memory property flags
		 */
		constexpr vk::MemoryPropertyFlags GetTargetMemoryProperty() const noexcept;

		/**
		 * @brief GetBuffer
		 * @return target buffer
		 */
		constexpr Buffer GetBuffer() noexcept;

		/**
		 * @brief GetVaultBuffer
		 * @return vault buffer
		 */
		constexpr Buffer GetVaultBuffer() noexcept;

		/**
		 * @brief ReceiveCommands
		 * @param regions regions that describe add/update/delete operations
		 * @return Commands structure for transfer or error result
		 */
		hrs::expected<Commands, vk::Result> ReceiveCommands(const DUAregions &regions);

		/**
		 * @brief IsCreated
		 * @return Checcks whether buffer is created or not
		 */
		constexpr bool IsCreated() const noexcept;

		/**
		 * @brief GetSize
		 * @return size of target buffer
		 */
		constexpr vk::DeviceSize GetSize() const noexcept;

		/**
		 * @brief GetVaultMemoryProperty
		 * @return vault buffer memory property flags
		 */
		constexpr vk::MemoryPropertyFlags GetVaultMemoryProperty() const noexcept;

		/**
		 * @brief DestroyVaultBuffer
		 *
		 * Explicitly destroys and frees vault buffer
		 */
		void DestroyVaultBuffer() noexcept;

	private:

		/**
		 * @brief transfer_adds
		 * @param commands Commands structure where new commands will be placed
		 * @param add_regions regions to add
		 * @param added_blocks free block buffer allocated blocks per each region
		 *
		 * Prepares transfer commands for add operations
		 */
		void transfer_adds(Commands &commands,
						   const std::vector<AddRegion> &add_regions,
						   const std::vector<hrs::block<vk::DeviceSize>> &added_blocks);

		/**
		 * @brief transfer_updates
		 * @param commands Commands structure where new commands will be placed
		 * @param update_regions regions to update
		 *
		 * Preapares transfer commands for update operations
		 */
		void transfer_updates(Commands &commands,
							  const std::vector<UpdateRegion> &update_regions);

		/**
		 * @brief allocate_buffer
		 * @param _allocator allocator that will be used for memory allocations
		 * @param _usage buffer usage for buffer create info
		 * @param _queue_family_index queue family index for bffer create info
		 * @param _size size of buffer
		 * @return allocated buffer with it's memory property flags or error result
		 */
		static hrs::expected<BufferWithProperty, vk::Result>
		allocate_buffer(Allocator *_allocator,
						vk::BufferUsageFlags _usage,
						std::uint32_t _queue_family_index,
						vk::DeviceSize _size);

	private:
		Allocator *allocator;///<allocator for buffer allocation
		BufferWithProperty buffer;///<target buffer
		BufferWithProperty vault_buffer;///<vault buffer
		hrs::free_block_allocator<vk::DeviceSize> blocks;///<free block allocator that manages free blocks
		vk::BufferUsageFlags usage;///<target buffer usage flags
		uint32_t queue_family_index;///<queue family index for buffer create info
	};

	FreeBlockBuffer::FreeBlockBuffer(Allocator *_allocator,
									 BufferWithProperty &&_buffer,
									 hrs::free_block_allocator<vk::DeviceSize> &&_blocks,
									 vk::BufferUsageFlags _usage,
									 uint32_t _queue_family_index)
		: allocator(_allocator),
		  buffer(std::move(_buffer)),
		  blocks(std::move(_blocks)),
		  usage(_usage),
		  queue_family_index(_queue_family_index){}

	hrs::expected<FreeBlockBuffer, vk::Result>
	FreeBlockBuffer::Create(Allocator *_allocator,
							vk::DeviceSize _alignnment,
							vk::DeviceSize _blocks_count,
							vk::BufferUsageFlags _usage,
							uint32_t _queue_family_index)
	{
		hrs::assert_true_debug(_allocator != nullptr, "Allocator isn't created yet!");
		hrs::assert_true_debug(hrs::is_power_of_two(_alignnment), "Alignment is not power of two!");
		hrs::free_block_allocator<vk::DeviceSize> free_block_alloc(_alignnment,
																   _blocks_count);

		auto allocated_buffer = allocate_buffer(_allocator, _usage, _queue_family_index, free_block_alloc.size());
		if(!allocated_buffer.has_value())
			return allocated_buffer.error();

		return FreeBlockBuffer(_allocator,
							   std::move(allocated_buffer.value()),
							   std::move(free_block_alloc),
							   _usage,
							   _queue_family_index);
	}

	FreeBlockBuffer::~FreeBlockBuffer()
	{
		if(IsCreated())
		{
			vk::Device device = allocator->GetDevice();
			if(vault_buffer.buffer_memory)
				vault_buffer.buffer_memory.Destroy(device);
			if(buffer.buffer_memory)
				buffer.buffer_memory.Destroy(device);
		}
	}

	FreeBlockBuffer::FreeBlockBuffer(FreeBlockBuffer &&buf) noexcept
		: allocator(buf.allocator),
		  buffer(std::move(buf.buffer)),
		  blocks(std::move(buf.blocks)),
		  usage(buf.usage),
		  queue_family_index(buf.queue_family_index)
	{
		buf.allocator = nullptr;
	}

	FreeBlockBuffer & FreeBlockBuffer::operator=(FreeBlockBuffer &&buf) noexcept
	{
		this->~FreeBlockBuffer();
		allocator = buf.allocator;
		buffer = std::move(buf.buffer);
		blocks = std::move(buf.blocks);
		usage = buf.usage;
		queue_family_index = buf.queue_family_index;
		buf.allocator = nullptr;
		return *this;
	}

	constexpr vk::MemoryPropertyFlags FreeBlockBuffer::GetTargetMemoryProperty() const noexcept
	{
		return buffer.prop_flags;
	}

	constexpr Buffer FreeBlockBuffer::GetBuffer() noexcept
	{
		return buffer.buffer_memory;
	}

	constexpr Buffer FreeBlockBuffer::GetVaultBuffer() noexcept
	{
		return vault_buffer.buffer_memory;
	}

	hrs::expected<Commands, vk::Result> FreeBlockBuffer::ReceiveCommands(const DUAregions &regions)
	{
		if(vault_buffer.buffer_memory)
			vault_buffer.buffer_memory.Destroy(allocator->GetDevice());

		for(const auto &del_region : regions.delete_regions)
			blocks.release_block(del_region);

		vk::DeviceSize appended_size = 0;
		std::vector<hrs::block<vk::DeviceSize>> added_blocks;
		added_blocks.reserve(regions.add_regions.size());
		for(const auto &add_region : regions.add_regions)
		{
			auto [app_block_count, app_block] = blocks.acquire_blocks(hrs::round_up_size_to_alignment(
																		  add_region.data_size,
																		  blocks.get_block_size()), 1);

			appended_size += app_block_count * blocks.get_block_size();
			added_blocks.push_back(app_block);
		}

		Commands commands;
		//just ignore vault for the first time!
		if(appended_size == 0)
		{
			//no allocation
			commands.commands_chain.reserve(regions.update_regions.size() + regions.add_regions.size());
			transfer_adds(commands, regions.add_regions, added_blocks);
			transfer_updates(commands, regions.update_regions);
			//commands.added_blocks = std::move(added_blocks);
			commands.state = Commands::BufferState::None;
		}
		else
		{
			//allocate
			auto allocated_buffer = allocate_buffer(allocator, usage, queue_family_index, blocks.size());
			if(!allocated_buffer.has_value())
				return allocated_buffer.error();

			vault_buffer = std::move(buffer);
			buffer = std::move(allocated_buffer.value());
			if(buffer.prop_flags & vk::MemoryPropertyFlagBits::eDeviceLocal)
				commands.state = Commands::BufferState::NewDeviceAllocation;
			else
				commands.state = Commands::BufferState::NewHostAllocation;

			auto copy_from_old = [&old_buffer = vault_buffer, &new_buffer = buffer]
				(const vk::CommandBuffer &buf) -> void
			{
				vk::BufferCopy copy(0, 0, old_buffer.buffer_memory.memory.size);
				buf.copyBuffer(old_buffer.buffer_memory.buffer,
							   new_buffer.buffer_memory.buffer,
							   copy);
			};

			if(regions.update_regions.empty())
			{
				//without updates
				commands.commands_chain.reserve(regions.add_regions.size() + 1);
				commands.commands_chain.push_back(copy_from_old);
				transfer_adds(commands, regions.add_regions, added_blocks);
			}
			else
			{
				//with updates
				commands.commands_chain.reserve(regions.add_regions.size() +
												regions.update_regions.size() + 2);
				commands.commands_chain.push_back(copy_from_old);
				transfer_adds(commands, regions.add_regions, added_blocks);
				auto updates_barrier = [&old_buffer = vault_buffer, &new_buffer = buffer]
					(const vk::CommandBuffer &buf) -> void
				{
					vk::BufferMemoryBarrier barrier(vk::AccessFlagBits::eTransferWrite,
													vk::AccessFlagBits::eTransferWrite,
													VK_QUEUE_FAMILY_IGNORED,
													VK_QUEUE_FAMILY_IGNORED,
													new_buffer.buffer_memory.buffer,
													0,
													old_buffer.buffer_memory.memory.size);

					buf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
										vk::PipelineStageFlagBits::eTransfer,
										{},
										{},
										barrier,
										{});
				};
				commands.commands_chain.push_back(updates_barrier);
				transfer_updates(commands, regions.update_regions);
			}
		}

		commands.added_blocks = std::move(added_blocks);
		return commands;
	}

	constexpr bool FreeBlockBuffer::IsCreated() const noexcept
	{
		return allocator != nullptr && buffer.buffer_memory.buffer;
	}

	constexpr vk::DeviceSize FreeBlockBuffer::GetSize() const noexcept
	{
		return blocks.size();
	}

	constexpr vk::MemoryPropertyFlags FreeBlockBuffer::GetVaultMemoryProperty() const noexcept
	{
		return vault_buffer.prop_flags;
	}

	void FreeBlockBuffer::DestroyVaultBuffer() noexcept
	{
		if(!IsCreated())
			return;

		vault_buffer.buffer_memory.Destroy(allocator->GetDevice());
		vault_buffer.prop_flags = {};
	}

	void FreeBlockBuffer::transfer_adds(Commands &commands,
										const std::vector<AddRegion> &add_regions,
										const std::vector<hrs::block<vk::DeviceSize>> &added_blocks)
	{
		if(add_regions.size() == 1)
		{
			vk::BufferCopy copy{0, added_blocks[0].offset, add_regions[0].data_size};
			TransferBufferRegions reg(buffer.buffer_memory.buffer,
									  copy,
									  add_regions[0].data_offset,
									  add_regions[0].data);

			commands.commands_chain.push_back(std::move(reg));
		}
		else
		{
			for(std::size_t i = 0; i < add_regions.size(); i++)
			{
				std::vector<vk::BufferCopy> copy;
				copy.push_back(vk::BufferCopy(0,
											  added_blocks[i].offset,
											  add_regions[i].data_size));

				std::vector<std::size_t> offset;
				offset.push_back(add_regions[i].data_offset);
				TransferBufferRegions regs(buffer.buffer_memory.buffer,
										   std::move(copy),
										   std::move(offset),
										   add_regions[i].data);

				commands.commands_chain.push_back(std::move(regs));
			}
		}
	}

	void FreeBlockBuffer::transfer_updates(Commands &commands,
										   const std::vector<UpdateRegion> &update_regions)
	{
		if(update_regions.size() == 1)
		{
			vk::BufferCopy copy{0,
								update_regions[0].update_region.offset,
								update_regions[0].update_region.size};

			TransferBufferRegions reg(buffer.buffer_memory.buffer,
									  copy,
									  update_regions[0].data_offset,
									  update_regions[0].data);

			commands.commands_chain.push_back(std::move(reg));
		}
		else
		{
			for(const auto &upd : update_regions)
			{
				std::vector<vk::BufferCopy> copy;
				copy.push_back(vk::BufferCopy(0,
											  upd.update_region.offset,
											  upd.update_region.size));

				std::vector<std::size_t> offset;
				offset.push_back(upd.data_offset);
				TransferBufferRegions regs(buffer.buffer_memory.buffer,
										   std::move(copy),
										   std::move(offset),
										   upd.data);

				commands.commands_chain.push_back(std::move(regs));
			}
		}
	}

	hrs::expected<BufferWithProperty, vk::Result>
	FreeBlockBuffer::allocate_buffer(Allocator *_allocator,
									 vk::BufferUsageFlags _usage,
									 std::uint32_t _queue_family_index,
									 vk::DeviceSize _size)
	{
		_usage |= vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
		vk::BufferCreateInfo buffer_info({},
										 _size,
										 _usage,
										 vk::SharingMode::eExclusive,
										 1,
										 &_queue_family_index);


		constexpr static std::array buffer_conds =
		{
			AllocateConditionalInfo{DesiredType::Only, vk::MemoryPropertyFlagBits::eDeviceLocal, false},
			AllocateConditionalInfo{DesiredType::Any, vk::MemoryPropertyFlagBits::eDeviceLocal, false},
			AllocateConditionalInfo{DesiredType::Any, vk::MemoryPropertyFlagBits::eHostVisible, false}
		};

		auto allocated_buffer = AllocateBuffer(*_allocator, buffer_conds, buffer_info);
		if(!allocated_buffer.has_value())
			return vk::Result::eErrorOutOfHostMemory;

		return BufferWithProperty{std::move(allocated_buffer->first), allocated_buffer->second->property_flags};
	}
};


