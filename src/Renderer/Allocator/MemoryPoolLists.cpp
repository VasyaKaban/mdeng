#include "MemoryPoolLists.h"
#include "MemoryPool.h"

namespace FireLand
{
	void MemoryPoolLists::Clear(VkDevice device, const AllocatorLoader &al, const VkAllocationCallbacks *alc) noexcept
	{
		for(auto &pool_list : pools)
			for(auto &pool : pool_list)
				pool.Destroy(device, al, alc);
	}

	bool MemoryPoolLists::IsEmpty() const noexcept
	{
		for(const auto &pool_list : pools)
			for(const auto &pool : pool_list)
				if(pool.IsEmpty())
					return true;

		return false;
	}

	MemoryPoolLists::PoolContainer & MemoryPoolLists::GetPools(MemoryPoolType type) noexcept
	{
		return pools[static_cast<std::size_t>(type)];
	}

	const MemoryPoolLists::PoolContainer & MemoryPoolLists::GetPools(MemoryPoolType type) const noexcept
	{
		return pools[static_cast<std::size_t>(type)];
	}

	MemoryPoolLists::Iterator MemoryPoolLists::Insert(MemoryPool &&pool) noexcept
	{
		PoolContainer *pc = &pools[static_cast<std::size_t>(pool.GetType())];
		pc->push_back(std::move(pool));
		return std::prev(pc->end());
	}

	void MemoryPoolLists::Rearrange(MemoryPoolType prev_type, Iterator it) noexcept
	{
		PoolContainer *prev_container = &pools[static_cast<std::size_t>(prev_type)];
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(*prev_container, it),
							   "Passed iterator is not a part of this list!");

		if(prev_type == it->GetType())
			return;

		PoolContainer *new_container = &pools[static_cast<std::size_t>(it->GetType())];
		new_container->splice(new_container->end(), *prev_container, it, std::next(it));
	}

	void MemoryPoolLists::Release(Iterator pool,
								  VkDevice device,
								  const AllocatorLoader &al,
								  const VkAllocationCallbacks *alc) noexcept
	{
		PoolContainer *pc = &pools[static_cast<std::size_t>(pool->GetType())];
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(*pc, pool),
							   "Passed iterator is not a part of this list!");

		pool->Destroy(device, al, alc);
		pc->erase(pool);
	}
};
