#include "IndexPool.h"
#include "DataIndexStorage.h"
#include "../../hrs/block.hpp"

namespace FireLand
{
	IndexPool::IndexPool(DataIndexStorage *_parent_stoarge,
						 std::uint32_t _init_size_power,
						 std::uint32_t _rounding_size)
		: parent_storage(_parent_stoarge),
		  pool_offset(0),
		  size(_rounding_size * _init_size_power),
		  pre_sync_size(size),
		  fillness(0),
		  rounding_size(_rounding_size),
		  is_sync_needed(false)
	{
		virtual_indices.reserve(size);
		index_subscribers.reserve(size);
	}

	IndexPool::IndexPool(IndexPool &&ip) noexcept
		: parent_storage(ip.parent_storage),
		  pool_offset(std::exchange(ip.pool_offset, 0)),
		  size(std::exchange(ip.size, 0)),
		  pre_sync_size(ip.pre_sync_size),
		  fillness(ip.fillness),
		  rounding_size(ip.rounding_size),
		  is_sync_needed(ip.is_sync_needed),
		  virtual_indices(std::move(ip.virtual_indices)),
		  index_subscribers(std::move(ip.index_subscribers))
	{

	}

	IndexPool & IndexPool::operator=(IndexPool &&ip) noexcept
	{
		parent_storage = ip.parent_storage;
		pool_offset = std::exchange(ip.pool_offset, 0);
		size = std::exchange(ip.size, 0);
		pre_sync_size = ip.pre_sync_size;
		fillness = ip.fillness;
		rounding_size = ip.rounding_size;
		is_sync_needed = ip.is_sync_needed;
		virtual_indices = std::move(ip.virtual_indices);
		index_subscribers = std::move(ip.index_subscribers);

		return *this;
	}

	void IndexPool::NewAddOp(std::uint32_t data_index, std::uint32_t *subscriber_ptr)
	{
		fillness++;
		if(fillness == size)
		{
			size = hrs::round_up_size_to_alignment(fillness, rounding_size);
			virtual_indices.reserve(size);
			index_subscribers.reserve(size);
		}

		virtual_indices.push_back(data_index);
		index_subscribers.push_back(subscriber_ptr);
		if(subscriber_ptr)
			*subscriber_ptr = virtual_indices.size() - 1;
		is_sync_needed = true;
	}

	void IndexPool::NewRemoveOp(std::uint32_t index) noexcept
	{
		fillness--;
		if(index != size)//non-back
		{
			std::swap(virtual_indices.back(), virtual_indices[index]);
			std::swap(index_subscribers.back(), index_subscribers[index]);
			virtual_indices.pop_back();
			index_subscribers.pop_back();

			if(index_subscribers[index])
				*index_subscribers[index] = index;
		}
		is_sync_needed = true;
	}

	bool IndexPool::IsSyncNeeded() const noexcept
	{
		return is_sync_needed;
	}

	bool IndexPool::HasData() const noexcept
	{
		return fillness != 0;
	}

	void IndexPool::Sync()
	{
		if(IsSyncNeeded())
			parent_storage->UpdatePool(this);
	}

	DataIndexStorage * IndexPool::GetParentStorage() noexcept
	{
		return parent_storage;
	}

	const DataIndexStorage * IndexPool::GetParentStorage() const noexcept
	{
		return parent_storage;
	}

	std::uint32_t IndexPool::GetSize() const noexcept
	{
		return size;
	}

	std::uint32_t IndexPool::GetPreSyncSize() const noexcept
	{
		return size;
	}

	vk::DeviceSize IndexPool::GetPoolOffset() const noexcept
	{
		return pool_offset;
	}

	const std::vector<std::uint32_t> & IndexPool::GetVirtualIndices() const noexcept
	{
		return virtual_indices;
	}

	void IndexPool::UpdateSize() noexcept
	{
		std::uint32_t rounded_fillness = hrs::round_up_size_to_alignment(fillness, rounding_size);
		size = std::max(size, rounded_fillness);
		is_sync_needed = true;
	}

	void IndexPool::UpdateOffset(vk::DeviceSize new_pool_offset) noexcept
	{
		pool_offset = new_pool_offset;
		pre_sync_size = size;
		is_sync_needed = false;
	}
};
