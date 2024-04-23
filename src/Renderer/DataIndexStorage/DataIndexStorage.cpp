#include <execution>
#include "DataIndexStorage.h"
#include "IndexPool.h"
#include "hrs/scoped_call.hpp"
#include "../Context/Device.h"
#include "../Allocator/AllocateFromMany.hpp"

namespace FireLand
{
	void DataIndexStorage::init(std::vector<BoundedBufferSize> &&_index_buffers,
								hrs::unsized_free_block_chain<vk::DeviceSize> &&_free_blocks)
	{
		index_buffers = std::move(_index_buffers);
		free_blocks = std::move(_free_blocks);
		actual_indices_mask = std::numeric_limits<decltype(actual_indices_mask)>::max();
	}

	DataIndexStorage::DataIndexStorage(Device *_parent_device,
									   std::uint32_t _rounding_indices_count,
									   const std::function<NewPoolSizeCalculator> &_calc) noexcept
		: parent_device(_parent_device),
		  actual_indices_mask(0),
		  rounding_indices_count(_rounding_indices_count),
		  calc(_calc) {}

	hrs::error DataIndexStorage::Recreate(std::uint32_t count, std::uint32_t init_indices_count)
	{
		if(count == 0 || init_indices_count == 0)
			return {};

		vk::DeviceSize buffer_size = hrs::round_up_size_to_alignment(init_indices_count,
																	 rounding_indices_count) *
									 sizeof(std::uint32_t);
		std::vector<BoundedBufferSize> _buffers;
		_buffers.reserve(count);
		hrs::scoped_call buffers_dtor([&_buffers, this]()
		{
			for(auto &buffer : _buffers)
				parent_device->GetAllocator()->Release(buffer.bounded_buffer, MemoryPoolOnEmptyPolicy::Free);
		});

		for(std::uint32_t i = 0; i < count; i++)
		{
			auto buffer_exp = allocate_buffer(buffer_size);
			if(!buffer_exp)
				return buffer_exp.error();

			_buffers.push_back(std::move(buffer_exp.value()));
		}

		hrs::unsized_free_block_chain<vk::DeviceSize> _free_blocks(buffer_size);
		init(std::move(_buffers), std::move(_free_blocks));

		buffers_dtor.drop();
		return {};
	}

	DataIndexStorage::~DataIndexStorage()
	{
		Destroy();
	}

	DataIndexStorage::DataIndexStorage(DataIndexStorage &&storage) noexcept
		: parent_device(storage.parent_device),
		  index_buffers(std::move(storage.index_buffers)),
		  actual_indices_mask(storage.actual_indices_mask),
		  rounding_indices_count(storage.rounding_indices_count),
		  free_blocks(std::move(storage.free_blocks)),
		  calc(storage.calc),
		  pending_adds(std::move(storage.pending_adds)),
		  pending_updates(std::move(storage.pending_updates)) {}

	DataIndexStorage & DataIndexStorage::operator=(DataIndexStorage &&storage) noexcept
	{
		Destroy();

		parent_device = storage.parent_device;
		index_buffers = std::move(storage.index_buffers);
		actual_indices_mask = storage.actual_indices_mask;
		rounding_indices_count = storage.rounding_indices_count;
		free_blocks = std::move(storage.free_blocks);
		calc = storage.calc;
		pending_adds = std::move(storage.pending_adds);
		pending_updates = std::move(storage.pending_updates);

		return *this;
	}

	void DataIndexStorage::Destroy()
	{
		if(!IsCreated())
			return;

		for(auto &buffer : index_buffers)
			parent_device->GetAllocator()->Release(buffer.bounded_buffer, MemoryPoolOnEmptyPolicy::Free);

		index_buffers.clear();
		free_blocks.clear();
		pending_adds.clear();
		pending_updates.clear();
	}

	bool DataIndexStorage::IsCreated() const noexcept
	{
		return !index_buffers.empty();
	}

	vk::DeviceSize DataIndexStorage::GetActualSize() const noexcept
	{
		return free_blocks.get_size();
	}

	Device * DataIndexStorage::GetParentDevice() noexcept
	{
		return parent_device;
	}

	const Device * DataIndexStorage::GetParentDevice() const noexcept
	{
		return parent_device;
	}

	const std::function<NewPoolSizeCalculator> & DataIndexStorage::GetNewPoolSizeCalculator() const noexcept
	{
		return calc;
	}

	void DataIndexStorage::AddPool(IndexPool *pool, bool copy_data)
	{
		pending_adds.push_back(AddPoolOp(pool, copy_data));
	}

	void DataIndexStorage::RemovePool(const IndexPool *pool)
	{
		const hrs::block<vk::DeviceSize> blk(pool->GetSize() * sizeof(std::uint32_t), pool->GetPoolOffset());
		free_blocks.release(blk);
	}

	void DataIndexStorage::UpdatePool(IndexPool *pool)
	{
		pending_updates.push_back(pool);
	}

	bool DataIndexStorage::IsSyncNeeded(std::uint32_t index) const noexcept
	{
		return
			is_actual(index) &&
			pending_adds.empty() &&
			pending_updates.empty();
	}

	hrs::error DataIndexStorage::SyncAndWrite(std::uint32_t index)
	{
		if(!IsSyncNeeded(index))
			return {};

		bool has_updates_or_adds = !pending_adds.empty() || !pending_updates.empty();

		//evaluate adds places
		for(auto &add : pending_adds)
		{
			add.pool->UpdateSize();
			const hrs::mem_req<vk::DeviceSize> req(add.pool->GetSize() * sizeof(std::uint32_t),
												   sizeof(std::uint32_t));
			auto blk = free_blocks.acquire(req).first;
			add.pool->UpdateOffset(blk.offset);
		}

		bool realloc = false;
		BoundedBufferSize new_buffer;
		std::uint32_t actual_index = get_actual_index(index);
		if(index_buffers[index].size < free_blocks.get_size())//reallocate
		{
			bool realloc = true;
			if(actual_index != index)
			{
				parent_device->GetAllocator()->Release(index_buffers[index].bounded_buffer,
													   MemoryPoolOnEmptyPolicy::Free);
				index_buffers[actual_index] = {};
			}

			vk::DeviceSize new_size =
				hrs::round_up_size_to_alignment(free_blocks.get_size() / sizeof(std::uint32_t),
												rounding_indices_count) * sizeof(std::uint32_t);

			auto buffer_exp = allocate_buffer(new_size);
			if(!buffer_exp)
				return buffer_exp.error();

			if(actual_index != index)
				index_buffers[index] = std::move(buffer_exp.value());
			else
				new_buffer = std::move(buffer_exp.value());

			free_blocks.increase_size(new_size - free_blocks.get_size());
		}

		/*
		 no realloc, target == actual -> no copy
		 no realloc, target != actual -> copy
		 realloc, target == actual -> copy
		 realloc, target != actual -> copy
		 */

		BoundedBufferSize &actual_buffer = index_buffers[get_actual_index(index)];
		if(!(!realloc && is_actual(index)))
		{
			//copy from actual
			if(actual_index != index)//copy to index_buffers[index]
			{
				std::copy_n(std::execution::unseq,
							actual_buffer.bounded_buffer.GetBufferMapPtr(),
							actual_buffer.size,
							index_buffers[index].bounded_buffer.GetBufferMapPtr());
			}
			else//copy to new_buffer
			{
				std::copy_n(std::execution::unseq,
							actual_buffer.bounded_buffer.GetBufferMapPtr(),
							actual_buffer.size,
							new_buffer.bounded_buffer.GetBufferMapPtr());

				parent_device->GetAllocator()->Release(index_buffers[index].bounded_buffer,
													   MemoryPoolOnEmptyPolicy::Free);
				index_buffers[index] = std::move(new_buffer);
			}
		}

		//copy from adds and updates
		for(auto &add : pending_adds)
		{
			if(!add.copy_data)
				continue;

			std::copy_n(std::execution::unseq,
						reinterpret_cast<const std::byte *>(add.pool->GetVirtualIndices().data()),
						add.pool->GetSize() * sizeof(std::uint32_t),
						index_buffers[index].bounded_buffer.GetBufferMapPtr() + add.pool->GetPoolOffset());
		}
		pending_adds.clear();

		for(auto &upd : pending_updates)
		{
			std::copy_n(std::execution::unseq,
						reinterpret_cast<const std::byte *>(upd->GetVirtualIndices().data()),
						upd->GetSize() * sizeof(std::uint32_t),
						index_buffers[index].bounded_buffer.GetBufferMapPtr() + upd->GetPoolOffset());
		}
		pending_updates.clear();

		//update indices
		if(has_updates_or_adds)
			actual_indices_mask = 0;

		actual_indices_mask |= (0x1 << index);
		return {};
	}

	vk::Buffer DataIndexStorage::GetBuffer(std::uint32_t index) const noexcept
	{
		return index_buffers[index].bounded_buffer.buffer;
	}

	bool DataIndexStorage::is_actual(std::uint32_t index) const noexcept
	{
		return actual_indices_mask & (0x1 << index);
	}

	std::uint32_t DataIndexStorage::get_actual_index(std::uint32_t index) const noexcept
	{
		if(actual_indices_mask == (0x1 << index))
			return index;

		using mask_type = decltype(actual_indices_mask);
		for(std::uint32_t  i = 0; i < std::numeric_limits<mask_type>::digits; i++)
		{
			if((actual_indices_mask & (0x1 << i)) && index != i)
				return i;
		}

		assert(false);
	}

	hrs::expected<BoundedBufferSize, hrs::error>
	DataIndexStorage::allocate_buffer(vk::DeviceSize size)
	{
		constexpr static std::array variants =
		{
			MemoryPropertyOpFlags(vk::MemoryPropertyFlagBits::eHostVisible,
								  MemoryTypeSatisfyOp::Any,
								  AllocationFlags::MapMemory),
			MemoryPropertyOpFlags(vk::MemoryPropertyFlagBits::eHostVisible,
								  MemoryTypeSatisfyOp::Any,
								  hrs::flags(AllocationFlags::MapMemory) |
									  AllocationFlags::AllowPlaceWithMixedResources),
		};

		const vk::BufferCreateInfo info({},
										size,
										vk::BufferUsageFlagBits::eStorageBuffer,
										vk::SharingMode::eExclusive);

		auto buffer_exp = AllocateFromMany(*parent_device->GetAllocator(),
										   variants,
										   info,
										   sizeof(std::uint32_t),
										   calc);

		if(!buffer_exp)
			return buffer_exp.error();

		return std::move(buffer_exp.value());
	}
};
