#include "MemoryPoolLists.h"
#include "MemoryPool.h"

namespace FireLand
{
	void MemoryPoolLists::Clear() noexcept
	{
		none_type_pools.clear();
		linear_type_pools.clear();
		non_linear_type_pools.clear();
		mixed_type_pools.clear();
	}

	bool MemoryPoolLists::IsEmpty() const noexcept
	{
		return none_type_pools.empty() &&
			   linear_type_pools.empty() &&
			   non_linear_type_pools.empty() &&
			   mixed_type_pools.empty();
	}

	MemoryPoolLists::PoolContainer & MemoryPoolLists::GetNoneTypePools() noexcept
	{
		return none_type_pools;
	}

	MemoryPoolLists::PoolContainer & MemoryPoolLists::GetLinearTypePools() noexcept
	{
		return linear_type_pools;
	}

	MemoryPoolLists::PoolContainer & MemoryPoolLists::GetNonLinearTypePools() noexcept
	{
		return non_linear_type_pools;
	}

	MemoryPoolLists::PoolContainer & MemoryPoolLists::GetMixedTypePools() noexcept
	{
		return mixed_type_pools;
	}

	MemoryPoolLists::PoolContainer & MemoryPoolLists::GetContainerBasedOnType(MemoryPoolType type) noexcept
	{
		switch(type)
		{
			case MemoryPoolType::None:
				return none_type_pools;
				break;
			case MemoryPoolType::Linear:
				return linear_type_pools;
				break;
			case MemoryPoolType::NonLinear:
				return non_linear_type_pools;
				break;
			case MemoryPoolType::Mixed:
				return mixed_type_pools;
				break;
		}
	}

	MemoryPoolLists::Iterator MemoryPoolLists::Insert(MemoryPool &&pool) noexcept
	{
		switch(pool.GetType())
		{
			case MemoryPoolType::None:
				none_type_pools.push_back(std::move(pool));
				return std::prev(none_type_pools.end());
				break;
			case MemoryPoolType::Linear:
				linear_type_pools.push_back(std::move(pool));
				return std::prev(linear_type_pools.end());
				break;
			case MemoryPoolType::NonLinear:
				non_linear_type_pools.push_back(std::move(pool));
				return std::prev(non_linear_type_pools.end());
				break;
			case MemoryPoolType::Mixed:
				mixed_type_pools.push_back(std::move(pool));
				return std::prev(mixed_type_pools.end());
				break;
		}
	}

	void MemoryPoolLists::Rearrange(MemoryPoolType prev_type, Iterator it) noexcept
	{
		PoolContainer *prev_container = nullptr;
		PoolContainer *new_container = nullptr;

		switch(prev_type)
		{
			case MemoryPoolType::None:
				prev_container = &none_type_pools;
				hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(none_type_pools, it),
									   "Passed iterator is not a part of this list!");
				break;
			case MemoryPoolType::Linear:
				prev_container = &linear_type_pools;
				hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(linear_type_pools, it),
									   "Passed iterator is not a part of this list!");
				break;
			case MemoryPoolType::NonLinear:
				prev_container = &non_linear_type_pools;
				hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(non_linear_type_pools, it),
									   "Passed iterator is not a part of this list!");
				break;
			case MemoryPoolType::Mixed:
				prev_container = &mixed_type_pools;
				hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(mixed_type_pools, it),
									   "Passed iterator is not a part of this list!");
				break;
		}

		if(prev_type == it->GetType())
			return;

		switch(it->GetType())
		{
			case MemoryPoolType::None:
				new_container = &none_type_pools;
				break;
			case MemoryPoolType::Linear:
				new_container = &linear_type_pools;
				break;
			case MemoryPoolType::NonLinear:
				new_container = &non_linear_type_pools;
				break;
			case MemoryPoolType::Mixed:
				new_container = &mixed_type_pools;
				break;
		}

		new_container->splice(new_container->end(), *prev_container, it, std::next(it));
	}

	void MemoryPoolLists::Release(Iterator pool) noexcept
	{
		switch(pool->GetType())
		{
			case MemoryPoolType::None:
				hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(none_type_pools, pool),
									   "Passed iterator is not a part of this list!");
				none_type_pools.erase(pool);
				break;
			case MemoryPoolType::Linear:
				hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(linear_type_pools, pool),
									   "Passed iterator is not a part of this list!");
				linear_type_pools.erase(pool);
				break;
			case MemoryPoolType::NonLinear:
				hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(non_linear_type_pools, pool),
									   "Passed iterator is not a part of this list!");
				non_linear_type_pools.erase(pool);
				break;
			case MemoryPoolType::Mixed:
				hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(mixed_type_pools, pool),
									   "Passed iterator is not a part of this list!");
				mixed_type_pools.erase(pool);
				break;
		}


	}
};
