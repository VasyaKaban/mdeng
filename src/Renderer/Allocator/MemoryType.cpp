#include "MemoryType.h"
#include "MemoryPool.h"

namespace FireLand
{
	vk::DeviceSize MemoryType::DefaultNewPoolSizeCalculator(vk::DeviceSize previous_size,
															vk::DeviceSize requested_size,
															const MemoryType &mem_type) noexcept
	{
		constexpr static vk::DeviceSize divisor = 32;
		if(previous_size == 0)//first call
		{
			const vk::MemoryHeap &mem_heap = mem_type.heap;
			vk::DeviceSize allocation_size = mem_heap.size / divisor;
			if(requested_size > allocation_size)
				allocation_size = (requested_size / allocation_size) * allocation_size + 1;

			if(allocation_size > mem_heap.size)
				return 0;

			return allocation_size;
		}
		else
		{
			previous_size = (previous_size >> 2) + (previous_size >> 4);
			if(previous_size < requested_size)
			{
				if(requested_size % divisor == 0)
					return requested_size;
				else if(requested_size < divisor)
					return 0;
				else
					return (requested_size / divisor) * divisor + 1;
					//do not think about allocations alignment(like power of two or smthing)
			}

			return previous_size;
		}
	}

	MemoryType::MemoryType(vk::Device _parent_device,
						   vk::MemoryHeap _heap,
						   vk::MemoryPropertyFlags _memory_property_flags,
						   std::uint32_t _index,
						   vk::DeviceSize _buffer_image_granularity) noexcept
		: parent_device(_parent_device),
		  heap(_heap),
		  memory_property_flags(_memory_property_flags),
		  index(_index),
		  buffer_image_granularity(_buffer_image_granularity)
	{
		hrs::assert_true_debug(parent_device, "Parent device isn't created yet!");
		hrs::assert_true_debug(hrs::is_power_of_two(_buffer_image_granularity),
							   "Buffer image granularity must be power of two!");
	}

	MemoryType::~MemoryType()
	{
		Destroy();
	}

	MemoryType::MemoryType(MemoryType &&mem_type) noexcept
		: parent_device(mem_type.parent_device),
		  heap(mem_type.heap),
		  memory_property_flags(mem_type.memory_property_flags),
		  index(mem_type.index),
		  buffer_image_granularity(mem_type.buffer_image_granularity),
		  lists(std::move(mem_type.lists)) {}

	MemoryType & MemoryType::operator=(MemoryType &&mem_type) noexcept
	{
		Destroy();

		parent_device = mem_type.parent_device;
		heap = mem_type.heap;
		memory_property_flags = mem_type.memory_property_flags;
		index = mem_type.index;
		buffer_image_granularity = mem_type.buffer_image_granularity;
		lists = std::move(mem_type.lists);

		return *this;
	}

	void MemoryType::Destroy()
	{
		lists.Clear();
	}

	bool MemoryType::IsSatisfy(const vk::MemoryRequirements &req) const noexcept
	{
		return (req.memoryTypeBits & GetMemoryTypeIndexMask()) == GetMemoryTypeIndexMask();
	}

	bool MemoryType::IsSatisfyAny(vk::MemoryPropertyFlags props) const noexcept
	{
		return (!props || static_cast<bool>(memory_property_flags & props));
	}

	bool MemoryType::IsSatisfyOnly(vk::MemoryPropertyFlags props) const noexcept
	{
		return (!props || memory_property_flags == props);
	}

	bool MemoryType::IsMappable() const noexcept
	{
		return static_cast<bool>(memory_property_flags & vk::MemoryPropertyFlagBits::eHostVisible);
	}

	bool MemoryType::IsEmpty() const noexcept
	{
		return lists.IsEmpty();
	}

	std::uint32_t MemoryType::GetMemoryTypeIndex() const noexcept
	{
		return index;
	}

	std::uint32_t MemoryType::GetMemoryTypeIndexMask() const noexcept
	{
		return (1 << index);
	}

#warning MAKE SURE THAT SIZE OF vk::*Resource*CreateInfo MUST BE THE ACCESSIBLE SIZE OF RESOURCE WHILE BLOCK BOUNDS IS ONLY FOR ALLOCATOR

	hrs::expected<BlockBindPool, hrs::error>
	MemoryType::Bind(vk::Buffer buffer,
					 const vk::MemoryRequirements &req,
					 hrs::flags<AllocationFlags> flags,
					 const std::function<NewPoolSizeCalculator> &calc)
	{
		hrs::assert_true_debug(buffer, "Buffer isn't created!");
		return bind(buffer, ResourceType::Linear, req, flags, calc);
	}

	hrs::expected<BlockBindPool, hrs::error>
	MemoryType::Bind(vk::Image image,
					 ResourceType res_type,
					 const vk::MemoryRequirements &req,
					 hrs::flags<AllocationFlags> flags,
					 const std::function<NewPoolSizeCalculator> &calc)
	{
		hrs::assert_true_debug(image, "Image isn't created!");
		return bind(image, res_type, req, flags, calc);
	}

	void MemoryType::Release(ResourceType res_type,
							 const BlockBindPool &bbp,
							 MemoryPoolOnEmptyPolicy policy)
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(lists.GetContainerBasedOnType(bbp.pool->GetType()),
																	bbp.pool),
							   "Passed pool is not part of this memory type!");

		MemoryPoolType prev_type = bbp.pool->GetType();
		bbp.pool->Release(res_type, bbp.block);
		lists.Rearrange(prev_type, bbp.pool);

		if(policy == MemoryPoolOnEmptyPolicy::Free)
		{
			if(bbp.pool->IsEmpty())
				lists.Release(bbp.pool);
		}
	}

	void MemoryType::Release(vk::Buffer buffer,
							 const BlockBindPool &bbp,
							 MemoryPoolOnEmptyPolicy policy)
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(lists.GetContainerBasedOnType(bbp.pool->GetType()),
																	bbp.pool),
							   "Passed pool is not part of this memory type!");

		MemoryPoolType prev_type = bbp.pool->GetType();
		bbp.pool->Release(buffer, bbp.block);
		lists.Rearrange(prev_type, bbp.pool);

		if(policy == MemoryPoolOnEmptyPolicy::Free)
		{
			if(bbp.pool->IsEmpty())
				lists.Release(bbp.pool);
		}
	}

	void MemoryType::Release(vk::Image image,
							 ResourceType res_type,
							 const BlockBindPool &bbp,
							 MemoryPoolOnEmptyPolicy policy)
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(lists.GetContainerBasedOnType(bbp.pool->GetType()),
																	bbp.pool),
							   "Passed pool is not part of this memory type!");

		MemoryPoolType prev_type = bbp.pool->GetType();
		bbp.pool->Release(image, res_type, bbp.block);
		lists.Rearrange(prev_type, bbp.pool);

		if(policy == MemoryPoolOnEmptyPolicy::Free)
		{
			if(bbp.pool->IsEmpty())
				lists.Release(bbp.pool);
		}
	}

	hrs::expected<BlockBindPool, hrs::error>
	MemoryType::bind(std::variant<vk::Buffer, vk::Image> resource,
					 ResourceType res_type,
					 const vk::MemoryRequirements &req,
					 hrs::flags<AllocationFlags> flags,
					 const std::function<NewPoolSizeCalculator> &calc)
	{
		hrs::assert_true_debug(IsSatisfy(req), "Memory type doesn't satisfy the memory requirements!");
		hrs::assert_true_debug(calc != nullptr, "New pool size calculator function is null pointer!");
		hrs::assert_true_debug(req.size > 0, "Size must be greater than zero!");
		hrs::assert_true_debug(hrs::is_power_of_two(req.alignment), "Alignment is not power ot two!");
		hrs::assert_true_debug((flags & AllocationFlags::MapMemory ? IsMappable() : true),
							   "Memory type is considered not to be mapped!");
		hrs::assert_true_debug((flags &
								AllocationFlags::AllocateSeparatePool &
								AllocationFlags::CreateOnExistedPools) ? false : true,
							   "Conflicting flags: AllocateSeparatePool and CreateOnExistedPools!");

#ifndef NDEBUG
		auto _req = (std::holds_alternative<vk::Buffer>(resource) ?
						 parent_device.getBufferMemoryRequirements(std::get<vk::Buffer>(resource)) :
						 parent_device.getImageMemoryRequirements(std::get<vk::Image>(resource)));

		hrs::assert_true_debug(_req.alignment <= req.alignment, "Wrong requirements alignment!");
		hrs::assert_true_debug(_req.size <= req.size, "Wrong requirements size!");
#endif

		if(req.size > heap.size)
		{
			if(memory_property_flags & vk::MemoryPropertyFlagBits::eDeviceLocal)
				return {vk::Result::eErrorOutOfDeviceMemory};
			else
				return {vk::Result::eErrorOutOfHostMemory};
		}

		if(flags & AllocationFlags::AllocateSeparatePool)//allocate new
		{
			auto alloc_exp = allocate_and_bind(resource, res_type, req, flags, calc);
			if(alloc_exp)
				return alloc_exp.value();
			else
				return {alloc_exp.error()};

		}
		else if(flags & AllocationFlags::CreateOnExistedPools)//create on existed
		{
			return bind_to_existed(resource, res_type, req, flags);
		}
		else//search for already created and then allocate if needed
		{
			auto bind_exp = bind_to_existed(resource, res_type, req, flags);
			if(bind_exp)
				return bind_exp.value();

			if(bind_exp.error().holds<vk::Result>())
				return {bind_exp.error().revive<vk::Result>()};

			auto alloc_exp = allocate_and_bind(resource, res_type, req, flags, calc);
			if(alloc_exp)
				return alloc_exp.value();
			else
				return {alloc_exp.error()};
		}
	}

	hrs::expected<hrs::block<vk::DeviceSize>, hrs::error>
	MemoryType::bind_to_pool(MemoryPool &pool,
							 std::variant<vk::Buffer, vk::Image> resource,
							 ResourceType res_type,
							 const vk::MemoryRequirements &req)
	{
		if(std::holds_alternative<vk::Buffer>(resource))
			return pool.Bind(std::get<vk::Buffer>(resource), {req.size, req.alignment});
		else
			return pool.Bind(std::get<vk::Image>(resource), res_type, {req.size, req.alignment});
	}

	hrs::expected<BlockBindPool, vk::Result>
	MemoryType::allocate_and_bind(std::variant<vk::Buffer, vk::Image> resource,
								  ResourceType res_type,
								  const vk::MemoryRequirements &req,
								  hrs::flags<AllocationFlags> flags,
								  const std::function<NewPoolSizeCalculator> &calc)
	{
		vk::DeviceSize size = calc(0, req.size, *this);
		MemoryPool pool(parent_device);
		vk::Result pool_res =
			(memory_property_flags & vk::MemoryPropertyFlagBits::eDeviceLocal ?
				 vk::Result::eErrorOutOfDeviceMemory :
				 vk::Result::eErrorOutOfHostMemory);
		while(size >= req.size)
		{
			pool_res = pool.Recreate(size,
									 index,
									 static_cast<bool>(flags & AllocationFlags::MapMemory),
									 buffer_image_granularity);

			if(pool_res != vk::Result::eSuccess)
			{
				if(pool_res == vk::Result::eErrorOutOfHostMemory || pool_res == vk::Result::eErrorOutOfDeviceMemory)
				{
					vk::DeviceSize prev_size = size;
					size = calc(size, req.size, *this);
					if(prev_size <= size)
						return pool_res;

					continue;
				}

				return pool_res;
			}

			auto bind_res = bind_to_pool(pool, resource, res_type, req);
			if(bind_res)
				return BlockBindPool(bind_res.value(), lists.Insert(std::move(pool)));

			hrs::assert_true_debug(!bind_res.error().holds<MemoryPoolResult>(), "BAD BIND RESULT!!!");
			pool.Destroy();
			return bind_res.error().revive<vk::Result>();
		}

		return pool_res;
	}

	hrs::expected<BlockBindPool, hrs::error>
	MemoryType::bind_to_existed(std::variant<vk::Buffer, vk::Image> resource,
								ResourceType res_type,
								const vk::MemoryRequirements &req,
								hrs::flags<AllocationFlags> flags)
	{
		auto bind_none_exp = try_bind_to_existed_pools(lists.GetNoneTypePools(),
													   MemoryPoolType::None,
													   resource,
													   res_type,
													   req);

		if(bind_none_exp)
			return bind_none_exp.value();

		if(bind_none_exp.error().holds<vk::Result>())
			return {bind_none_exp.error().revive<vk::Result>()};

		MemoryPoolLists::PoolContainer &same_type_container =
			(res_type == ResourceType::Linear ? lists.GetLinearTypePools() : lists.GetNonLinearTypePools());

		auto bind_same_exp = try_bind_to_existed_pools(same_type_container,
													   ResourceTypeToMemoryPoolType(res_type),
													   resource,
													   res_type,
													   req);

		if(bind_same_exp)
			return bind_same_exp.value();

		if(bind_same_exp.error().holds<vk::Result>())
			return {bind_same_exp.error().revive<vk::Result>()};

		if(flags & AllocationFlags::AllowPlaceWithMixedResources)
		{
			auto bind_mixed_exp = try_bind_to_existed_pools(lists.GetMixedTypePools(),
															MemoryPoolType::Mixed,
															resource,
															res_type,
															req);

			if(bind_mixed_exp)
				return bind_mixed_exp.value();

			if(bind_mixed_exp.error().holds<vk::Result>())
				return {bind_mixed_exp.error().revive<vk::Result>()};


			MemoryPoolLists::PoolContainer &opposite_type_container =
				(res_type == ResourceType::Linear ? lists.GetNonLinearTypePools() : lists.GetLinearTypePools());

			MemoryPoolType opposite_type =
				(res_type == ResourceType::Linear ? MemoryPoolType::NonLinear : MemoryPoolType::Linear);

			auto bind_opposite_exp = try_bind_to_existed_pools(opposite_type_container,
															   opposite_type,
															   resource,
															   res_type,
															   req);

			if(bind_opposite_exp)
				return bind_opposite_exp.value();

			if(bind_opposite_exp.error().holds<vk::Result>())
				return {bind_opposite_exp.error().revive<vk::Result>()};
		}

		return {MemoryPoolResult::NotEnoughSpace};
	}

	hrs::expected<BlockBindPool, hrs::error>
	MemoryType::try_bind_to_existed_pools(MemoryPoolLists::PoolContainer &pools,
										  MemoryPoolType pool_type,
										  std::variant<vk::Buffer, vk::Image> resource,
										  ResourceType res_type,
										  const vk::MemoryRequirements &req)
	{
		for(auto pool_it = pools.begin(); pool_it != pools.end(); pool_it++)
		{
			auto bind_exp = bind_to_pool(*pool_it, resource, res_type, req);
			if(bind_exp)
			{
				lists.Rearrange(pool_type, pool_it);
				return BlockBindPool(bind_exp.value(), pool_it);
			}

			if(!bind_exp.error().holds<MemoryPoolResult>())
				return {bind_exp.error().revive<vk::Result>()};
		}

		return {MemoryPoolResult::NotEnoughSpace};
	}
};
