#pragma once

#include <list>
#include "../../hrs/non_creatable.hpp"

namespace FireLand
{
	class MemoryPool;
	enum class ResourceType;
	enum class MemoryPoolType;

	class MemoryPoolLists : public hrs::non_copyable
	{
	public:
		using PoolContainer = std::list<MemoryPool>;
		using Iterator = PoolContainer::iterator;

		MemoryPoolLists() = default;
		~MemoryPoolLists() = default;
		MemoryPoolLists(MemoryPoolLists &&l) = default;
		MemoryPoolLists & operator=(MemoryPoolLists &&l) = default;

		void Clear() noexcept;
		bool IsEmpty() const noexcept;

		PoolContainer & GetNoneTypePools() noexcept;
		PoolContainer & GetLinearTypePools() noexcept;
		PoolContainer & GetNonLinearTypePools() noexcept;
		PoolContainer & GetMixedTypePools() noexcept;

		PoolContainer & GetContainerBasedOnType(MemoryPoolType type) noexcept;

		Iterator Insert(MemoryPool &&pool) noexcept;
		void Rearrange(MemoryPoolType prev_type, Iterator it) noexcept;
		void Release(Iterator pool) noexcept;

	private:
		PoolContainer none_type_pools;
		PoolContainer linear_type_pools;
		PoolContainer non_linear_type_pools;
		PoolContainer mixed_type_pools;
	};
};
