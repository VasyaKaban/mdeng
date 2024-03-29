#pragma once

#include "DescriptorPool.h"
#include "DescriptorStorage.h"
#include "../Context/Device.h"

namespace FireLand
{
	DescriptorPool::DescriptorPool(DescriptorStorage *_parent_storage,
								   vk::DescriptorPool _pool,
								   std::uint32_t _max_sets_count,
								   std::uint32_t _issued_sets_count) noexcept
		: parent_storage(_parent_storage),
		  pool(_pool),
		  max_sets_count(_max_sets_count),
		  issued_sets_count(_issued_sets_count) {}

	hrs::expected<DescriptorPool, vk::Result>
	DescriptorPool::Create(DescriptorStorage *_parent_storage, const vk::DescriptorPoolCreateInfo &info) noexcept
	{
		if(!_parent_storage)
			return DescriptorPool(_parent_storage, VK_NULL_HANDLE, 0, 0);

		auto [_pool_res, _pool] = _parent_storage->GetParentDevice()->GetHandle().createDescriptorPool(info);
		if(_pool_res != vk::Result::eSuccess)
			return _pool_res;

		return DescriptorPool(_parent_storage, _pool, info.maxSets, 0);
	}

	DescriptorPool::~DescriptorPool()
	{
		Destroy();
	}

	DescriptorPool::DescriptorPool(DescriptorPool &&d_pool) noexcept
		: parent_storage(d_pool.parent_storage),
		  pool(std::exchange(d_pool.pool, VK_NULL_HANDLE)),
		  free_sets(std::move(d_pool.free_sets)),
		  max_sets_count(d_pool.max_sets_count),
		  issued_sets_count(d_pool.issued_sets_count) {}

	DescriptorPool & DescriptorPool::operator=(DescriptorPool &&d_pool) noexcept
	{
		Destroy();

		parent_storage = d_pool.parent_storage;
		pool = std::exchange(d_pool.pool, VK_NULL_HANDLE);
		free_sets = std::move(d_pool.free_sets);
		max_sets_count = d_pool.max_sets_count;
		issued_sets_count = d_pool.issued_sets_count;

		return *this;
	}

	void DescriptorPool::Destroy()
	{
		if(!IsCreated())
			return;

		vk::Device device_handle = parent_storage->GetParentDevice()->GetHandle();
		device_handle.resetDescriptorPool(pool);
		pool = (device_handle.destroy(pool), VK_NULL_HANDLE);
		free_sets.clear();
		issued_sets_count = 0;
	}

	bool DescriptorPool::IsCreated() const noexcept
	{
		return pool;
	}

	hrs::expected<std::vector<vk::DescriptorSet>, vk::Result>
	DescriptorPool::AllocateSets(std::uint32_t count)
	{
		const std::vector<vk::DescriptorSetLayout> layouts(count, parent_storage->GetDescriptorSetLayout());
		const vk::DescriptorSetAllocateInfo info(pool, layouts);
		auto [sets_res, sets] = parent_storage->GetParentDevice()->GetHandle().allocateDescriptorSets(info);
		if(sets_res != vk::Result::eSuccess)
			return sets_res;

		issued_sets_count += count;
		return sets;
	}

	hrs::expected<std::vector<vk::DescriptorSet>, vk::Result>
	DescriptorPool::AllocateSets(std::span<const vk::DescriptorSetLayout> layouts)
	{
		const vk::DescriptorSetAllocateInfo info(pool, layouts);
		auto [sets_res, sets] = parent_storage->GetParentDevice()->GetHandle().allocateDescriptorSets(info);
		if(sets_res != vk::Result::eSuccess)
			return sets_res;

		issued_sets_count += layouts.size();
		return sets;
	}

	void DescriptorPool::ResetPool() noexcept
	{
		if(IsCreated())
		{
			parent_storage->GetParentDevice()->GetHandle().resetDescriptorPool(pool);
			free_sets.clear();
			issued_sets_count = 0;
		}
	}

	void DescriptorPool::RetireSets(std::span<const vk::DescriptorSet> sets)
	{
		if(sets.empty())
			return;

		if(IsCreated())
		{
			free_sets.insert(free_sets.end(), sets.begin(), sets.end());
			issued_sets_count -= sets.size();
		}
	}

	void DescriptorPool::FreeSets(std::span<const vk::DescriptorSet> sets) noexcept
	{
		if(sets.empty())
			return;

		if(IsCreated())
		{
			parent_storage->GetParentDevice()->GetHandle().freeDescriptorSets(pool, sets);
			issued_sets_count -= sets.size();
		}
	}

	DescriptorStorage * DescriptorPool::GetParentStorage() noexcept
	{
		return parent_storage;
	}

	const DescriptorStorage * DescriptorPool::GetParentStorage() const noexcept
	{
		return parent_storage;
	}

	vk::DescriptorPool DescriptorPool::GetHandle() const noexcept
	{
		return pool;
	}

	std::uint32_t DescriptorPool::GetMaxSetCount() const noexcept
	{
		return max_sets_count;
	}

	std::uint32_t DescriptorPool::GetIssuedSetCount() const noexcept
	{
		return issued_sets_count;
	}

	std::uint32_t DescriptorPool::GetFreeSetCount() const noexcept
	{
		return free_sets.size();
	}
};
