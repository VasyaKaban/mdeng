#include "MemoryPool.h"

namespace FireLand
{
	void MemoryPool::init(Memory &&_memory,
						  vk::DeviceSize _buffer_image_granularity,
						  hrs::sized_free_block_chain<vk::DeviceSize> &&_free_blocks) noexcept
	{
		memory = std::move(_memory);
		buffer_image_granularity = _buffer_image_granularity;
		free_blocks = std::move(_free_blocks);

		non_linear_object_count = 0;
		linear_object_count = 0;
	}

	MemoryPool::MemoryPool(vk::Device _parent_device) noexcept
		: parent_device(_parent_device)
	{
		hrs::assert_true_debug(_parent_device, "Parent device isn't created yet!");
	}

	MemoryPool::~MemoryPool()
	{
		Destroy();
	}

	MemoryPool::MemoryPool(MemoryPool &&pool) noexcept
		: parent_device(pool.parent_device),
		  buffer_image_granularity(pool.buffer_image_granularity),
		  non_linear_object_count(pool.non_linear_object_count),
		  linear_object_count(pool.linear_object_count),
		  memory(std::move(pool.memory)),
		  free_blocks(std::move(pool.free_blocks)) {}

	MemoryPool & MemoryPool::operator=(MemoryPool &&pool) noexcept
	{
		Destroy();

		parent_device = pool.parent_device;
		buffer_image_granularity = pool.buffer_image_granularity;
		non_linear_object_count = pool.non_linear_object_count;
		linear_object_count = pool.linear_object_count;
		memory = std::move(pool.memory);
		free_blocks = std::move(pool.free_blocks);

		return *this;
	}

	vk::Result MemoryPool::Recreate(vk::DeviceSize size,
									std::uint32_t memory_type_index,
									bool map_memory,
									vk::DeviceSize _buffer_image_granularity)
	{
		if(size == 0)
			return vk::Result::eSuccess;

		hrs::assert_true_debug(hrs::is_power_of_two(_buffer_image_granularity),
							   "Buffer image granularity is not power of two!");

		const vk::MemoryAllocateInfo info(size, memory_type_index);
		auto [_memory_res, _memory] = parent_device.allocateMemory(info);
		if(_memory_res != vk::Result::eSuccess)
			return _memory_res;

		Memory memory_obj(_memory, size, nullptr);
		if(map_memory)
		{
			vk::Result map_res = memory_obj.MapMemory(parent_device);
			if(map_res != vk::Result::eSuccess)
			{
				memory_obj.Free(parent_device);
				return map_res;
			}
		}

		init(std::move(memory_obj), _buffer_image_granularity, {size, 0});

		return vk::Result::eSuccess;
	}

	bool MemoryPool::IsCreated() const noexcept
	{
		return memory.IsAllocated();
	}

	void MemoryPool::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		memory.Free(parent_device);
		free_blocks.clear();
	}

	Memory & MemoryPool::GetMemory() noexcept
	{
		return memory;
	}

	const Memory & MemoryPool::GetMemory() const noexcept
	{
		return memory;
	}

	vk::Device MemoryPool::GetParentDevice() const noexcept
	{
		return parent_device;
	}

	vk::DeviceSize MemoryPool::GetGranularity() const noexcept
	{
		return buffer_image_granularity;
	}

	MemoryPoolType MemoryPool::GetType() const noexcept
	{
		if(IsGranularityFree())
			return MemoryPoolType::None;

		if(linear_object_count == 0 && non_linear_object_count == 0)
			return MemoryPoolType::None;
		else if(linear_object_count != 0 && non_linear_object_count == 0)
			return MemoryPoolType::Linear;
		else if(linear_object_count == 0 && non_linear_object_count != 0)
			return MemoryPoolType::NonLinear;
		else
			return MemoryPoolType::Mixed;
	}

	bool MemoryPool::IsGranularityFree() const noexcept
	{
		return buffer_image_granularity == 1;
	}

	bool MemoryPool::IsEmpty() const noexcept
	{
		return free_blocks.is_empty();
	}

	std::size_t MemoryPool::GetNonLinearObjectCount() const noexcept
	{
		return non_linear_object_count;
	}

	std::size_t MemoryPool::GetLinearObjectCount() const noexcept
	{
		return linear_object_count;
	}

	vk::ResultValue<std::byte *> MemoryPool::MapMemory() noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Memory pool isn't created yet!");

		vk::Result map_res = memory.MapMemory(parent_device);
		return {map_res, memory.GetMapPtr()};
	}

	void MemoryPool::Release(ResourceType res_type, const hrs::block<vk::DeviceSize> &blk)
	{
		hrs::assert_true_debug(IsCreated(), "Memory pool isn't created yet!");
		free_blocks.release(blk);
		dec_count(res_type);
	}

	void MemoryPool::Release(vk::Buffer buffer, const hrs::block<vk::DeviceSize> &blk)
	{
		hrs::assert_true_debug(IsCreated(), "Memory pool isn't created yet!");
		hrs::assert_true_debug(buffer, "Buffer isn't created!");

		free_blocks.release(blk);
		parent_device.destroy(buffer);

		dec_count(ResourceType::Linear);

		//if(free_blocks.is_empty())
		//	type = MemoryPoolType::None;
	}

	void MemoryPool::Release(vk::Image image, ResourceType res_type, const hrs::block<vk::DeviceSize> &blk)
	{
		hrs::assert_true_debug(IsCreated(), "Memory pool isn't created yet!");
		hrs::assert_true_debug(image, "Image isn't created!");

		free_blocks.release(blk);
		parent_device.destroy(image);

		dec_count(res_type);

		//if(free_blocks.is_empty())
		//	type = MemoryPoolType::None;
	}

	hrs::expected<hrs::block<vk::DeviceSize>, hrs::error>
	MemoryPool::Bind(vk::Buffer buffer,
					 hrs::mem_req<vk::DeviceSize> req) noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Memory pool isn't created yet!");
		hrs::assert_true_debug(buffer, "Buffer isn't created yet!");

		auto blk_opt = acquire_block_based_on_granularity(ResourceType::Linear, req);
		if(!blk_opt)
			return {MemoryPoolResult::NotEnoughSpace};

		vk::Result bind_res = parent_device.bindBufferMemory(buffer, memory.GetDeviceMemory(), blk_opt.value().offset);
		if(bind_res != vk::Result::eSuccess)
		{
			free_blocks.release(blk_opt.value());
			return {bind_res};
		}

		inc_count(ResourceType::Linear);

		return blk_opt.value();
	}

	hrs::expected<hrs::block<vk::DeviceSize>, hrs::error>
	MemoryPool::Bind(vk::Image image,
					 ResourceType res_type,
					 hrs::mem_req<vk::DeviceSize> req) noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Memory pool isn't created yet!");
		hrs::assert_true_debug(image, "Image isn't created yet!");

		auto blk_opt = acquire_block_based_on_granularity(res_type, req);
		if(!blk_opt)
			return {MemoryPoolResult::NotEnoughSpace};

		vk::Result bind_res = parent_device.bindImageMemory(image, memory.GetDeviceMemory(), blk_opt.value().offset);
		if(bind_res != vk::Result::eSuccess)
		{
			free_blocks.release(blk_opt.value());
			return {bind_res};
		}

		inc_count(res_type);

		return blk_opt.value();
	}

	void MemoryPool::inc_count(ResourceType res_type) noexcept
	{
		(res_type == ResourceType::Linear ? linear_object_count : non_linear_object_count)++;
	}

	void MemoryPool::dec_count(ResourceType res_type) noexcept
	{
		std::size_t &object_counter = (res_type == ResourceType::Linear ? linear_object_count : non_linear_object_count);
		hrs::assert_true_debug(object_counter != 0,
							   "Release operation with resource cannot be performed"
							   " because counter of target resource type objects is already zero!");
		object_counter--;
	}

	std::optional<hrs::block<vk::DeviceSize>>
	MemoryPool::acquire_block_based_on_granularity(ResourceType res_type,
												   hrs::mem_req<vk::DeviceSize> req)
	{
		MemoryPoolType type = GetType();
		const bool can_be_acquired_without_granularity_use =
			(IsGranularityFree() ||
			 type == MemoryPoolType::None ||
			 (type == ResourceTypeToMemoryPoolType(res_type)));

		if(can_be_acquired_without_granularity_use)
			return free_blocks.acquire(req.size, req.alignment);

		req.alignment = std::max(req.alignment, buffer_image_granularity);
		vk::DeviceSize upper_bound_size = hrs::round_up_size_to_alignment(req.size, buffer_image_granularity);

		/*We use upper_bound_size because we must to find next bound of page and
		 existed resources cannot alias with new resource

		 Ex: assume that granularity is 4 bytes

		 new resource: size = 9, alignment = 2
		 granularity_fixed_alignment = 4
		 granularity_fixed_size = 12

		   nnnnnnnnn -> resource without granularity
		 p***p***p***p***p***p
		 rrr           rrrrrr--+
					   |	   |
			 nnnnnnnnn000------+--- aliasing(we don't know type of existed resource
					   |_______|		and it's also must be placed with respect to granularity,
										even if the whole new resource fits the free space!)
		 */
		for(auto hint_it = free_blocks.begin(); hint_it != free_blocks.end(); hint_it++)
		{
			if(free_blocks.is_block_can_be_placed(hint_it, upper_bound_size, req.alignment))
				return free_blocks.acquire_by_hint(hint_it, {req.size, req.alignment});
		}

		return {};
	}
};
