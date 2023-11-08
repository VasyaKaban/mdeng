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

namespace FireLand
{
	struct UpdateRegion
	{
		std::byte *data;
		hrs::block<vk::DeviceSize> update_region;
		vk::DeviceSize data_offset;
	};

	struct AddRegion
	{
		std::byte *data;
		vk::DeviceSize data_offset;
		vk::DeviceSize data_size;
	};

	struct DUAregions
	{
		std::vector<hrs::block<vk::DeviceSize>> delete_regions;
		std::vector<UpdateRegion> update_regions;
		std::vector<AddRegion> add_regions;
	};

	struct Commands
	{
		enum BufferState
		{
			None,
			NewDeviceAllocation,
			NewHostAllocation
		} state;

		std::vector<hrs::block<vk::DeviceSize>> added_blocks;
		std::vector<TransferObject> commands_chain;
	};

	struct BufferWithProperty
	{
		Buffer buffer_memory;
		vk::MemoryPropertyFlags prop_flags;

		BufferWithProperty() = default;
		BufferWithProperty(BufferWithProperty &&buf) noexcept = default;
		BufferWithProperty(Buffer &&_buffer_memory, vk::MemoryPropertyFlags _prop_flags)
			: buffer_memory(std::move(_buffer_memory)), prop_flags(_prop_flags) {}

		constexpr auto operator=(BufferWithProperty &&buf) noexcept -> BufferWithProperty & = default;
	};

	class FreeBlockBuffer
	{
	private:
		FreeBlockBuffer(Allocator *_allocator,
						BufferWithProperty &&_buffer,
						hrs::free_block_allocator<vk::DeviceSize> &&_blocks,
						vk::BufferUsageFlags _usage,
						uint32_t _queue_family_index);
	public:
		static auto Create(Allocator *_allocator,
						   vk::DeviceSize _alignnment,
						   vk::DeviceSize _blocks_count,
						   vk::BufferUsageFlags _usage,
						   uint32_t _queue_family_index) -> hrs::expected<FreeBlockBuffer, vk::Result>;

		~FreeBlockBuffer();
		FreeBlockBuffer(const FreeBlockBuffer &) = delete;
		FreeBlockBuffer(FreeBlockBuffer &&buf) noexcept;
		auto operator=(const FreeBlockBuffer &) -> FreeBlockBuffer & = delete;
		auto operator=(FreeBlockBuffer &&buf) noexcept -> FreeBlockBuffer &;

		constexpr auto GetTargetMemoryProperty() const noexcept -> vk::MemoryPropertyFlags;
		constexpr auto GetBuffer() noexcept -> Buffer;
		constexpr auto GetVaultBuffer() noexcept -> Buffer;

		auto ReceiveCommands(const DUAregions &regions) -> hrs::expected<Commands, vk::Result>;
		constexpr auto IsCreated() const noexcept -> bool;
		constexpr auto GetSize() const noexcept -> vk::DeviceSize;
		constexpr auto GetVaultMemoryProperty() const noexcept -> vk::MemoryPropertyFlags;
		auto DestroyVaultBuffer() noexcept -> void;

	private:
		auto transfer_adds(Commands &commands,
						   const std::vector<AddRegion> &add_regions,
						   const std::vector<hrs::block<vk::DeviceSize>> &added_blocks) -> void;

		auto transfer_updates(Commands &commands,
							  const std::vector<UpdateRegion> &update_regions) -> void;

		static auto allocate_buffer(Allocator *_allocator,
									vk::BufferUsageFlags _usage,
									std::uint32_t _queue_family_index,
									vk::DeviceSize _size) -> hrs::expected<BufferWithProperty, vk::Result>;

	private:
		Allocator *allocator;
		BufferWithProperty buffer;
		BufferWithProperty vault_buffer;
		hrs::free_block_allocator<vk::DeviceSize> blocks;
		vk::BufferUsageFlags usage;
		uint32_t queue_family_index;
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

	auto FreeBlockBuffer::Create(Allocator *_allocator,
								 vk::DeviceSize _alignnment,
								 vk::DeviceSize _blocks_count,
								 vk::BufferUsageFlags _usage,
								 uint32_t _queue_family_index) -> hrs::expected<FreeBlockBuffer, vk::Result>
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

	auto FreeBlockBuffer::operator=(FreeBlockBuffer &&buf) noexcept -> FreeBlockBuffer &
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

	constexpr auto FreeBlockBuffer::GetTargetMemoryProperty() const noexcept -> vk::MemoryPropertyFlags
	{
		return buffer.prop_flags;
	}

	constexpr auto FreeBlockBuffer::GetBuffer() noexcept -> Buffer
	{
		return buffer.buffer_memory;
	}

	constexpr auto FreeBlockBuffer::GetVaultBuffer() noexcept -> Buffer
	{
		return vault_buffer.buffer_memory;
	}

	auto FreeBlockBuffer::ReceiveCommands(const DUAregions &regions) -> hrs::expected<Commands, vk::Result>
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
															  blocks.get_block_size(),
															  add_region.data_size), 1);

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
				vk::BufferCopy copy(0, 0, old_buffer.buffer_memory.size);
				buf.copyBuffer(old_buffer.buffer_memory.buffer,
							   new_buffer.buffer_memory.buffer,
							   copy);
			};

			if(regions.update_regions.empty())
			{
				//w/o updates
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
													old_buffer.buffer_memory.size);

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

	constexpr auto FreeBlockBuffer::IsCreated() const noexcept -> bool
	{
		return allocator != nullptr && buffer.buffer_memory.buffer;
	}

	constexpr auto FreeBlockBuffer::GetSize() const noexcept -> vk::DeviceSize
	{
		return blocks.size();
	}

	constexpr auto FreeBlockBuffer::GetVaultMemoryProperty() const noexcept -> vk::MemoryPropertyFlags
	{
		return vault_buffer.prop_flags;
	}

	auto FreeBlockBuffer::DestroyVaultBuffer() noexcept -> void
	{
		if(!IsCreated())
			return;

		vault_buffer.buffer_memory.Destroy(allocator->GetDevice());
		vault_buffer.prop_flags = {};
		vault_buffer.buffer_memory.buffer = nullptr;
		vault_buffer.buffer_memory.memory = nullptr;
		vault_buffer.buffer_memory.map_ptr = nullptr;
		vault_buffer.buffer_memory.size = 0;
	}

	auto FreeBlockBuffer::transfer_adds(Commands &commands,
										const std::vector<AddRegion> &add_regions,
										const std::vector<hrs::block<vk::DeviceSize>> &added_blocks) -> void
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

	auto FreeBlockBuffer::transfer_updates(Commands &commands,
										   const std::vector<UpdateRegion> &update_regions) -> void
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

	auto FreeBlockBuffer::allocate_buffer(Allocator *_allocator,
										  vk::BufferUsageFlags _usage,
										  std::uint32_t _queue_family_index,
										  vk::DeviceSize _size) -> hrs::expected<BufferWithProperty, vk::Result>
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
			AllocateConditionlInfo{DesiredType::Only, vk::MemoryPropertyFlagBits::eDeviceLocal, false},
			AllocateConditionlInfo{DesiredType::Any, vk::MemoryPropertyFlagBits::eDeviceLocal, false},
			AllocateConditionlInfo{DesiredType::Any, vk::MemoryPropertyFlagBits::eHostVisible, false}
		};

		auto allocated_buffer = AllocateBuffer(*_allocator, buffer_conds, buffer_info);
		if(!allocated_buffer.has_value())
			return vk::Result::eErrorOutOfHostMemory;

		return BufferWithProperty{std::move(allocated_buffer->first), allocated_buffer->second->property_flags};
	}
};


