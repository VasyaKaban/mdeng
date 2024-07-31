#pragma once

#include <list>
#include "hrs/non_creatable.hpp"
#include "MemoryPool.h"

namespace FireLand
{
	class MemoryPoolLists
		: public hrs::non_copyable,
		  public hrs::non_move_assignable
	{
	public:
		using PoolContainer = std::list<MemoryPool>;
		using Iterator = PoolContainer::iterator;

		MemoryPoolLists() = default;
		~MemoryPoolLists() = default;
		MemoryPoolLists(MemoryPoolLists &&l) = default;

		void Clear(VkDevice device, const AllocatorLoader &al, const VkAllocationCallbacks *alc) noexcept;
		bool IsEmpty() const noexcept;

		PoolContainer & GetPools(MemoryPoolType type) noexcept;
		const PoolContainer & GetPools(MemoryPoolType type) const noexcept;

		Iterator Insert(MemoryPool &&pool) noexcept;
		void Rearrange(MemoryPoolType prev_type, Iterator it) noexcept;
		void Release(Iterator pool,
					 VkDevice device,
					 const AllocatorLoader &al,
					 const VkAllocationCallbacks *alc) noexcept;

	private:
		std::array<PoolContainer,
				   static_cast<std::size_t>(MemoryPoolType::MemoryPoolTypeMaxUnused)> pools;
	};
};
