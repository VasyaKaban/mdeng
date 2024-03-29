#pragma once

#include "../../Context/Device.h"
#include "../../Allocator/BoundedResourceSize.hpp"
#include "../../../hrs/unexpected_result.hpp"
#include "../../Allocator/AllocateFromMany.hpp"

namespace FireLand
{
	template<typename C>
	concept BufferPoolCreator = requires
	{
		{C::GetBlockSizePower()} -> std::same_as<vk::DeviceSize>;
		{C::GetBufferUsage()} -> std::same_as<vk::BufferUsageFlags>;
		{C::GetAllocationVariants()} -> std::convertible_to<std::span<const MemoryPropertyOpFlags>>;
		{C::GetNewPoolSizeCalculator()} -> std::same_as<NewPoolSizeCalculator>;
	};

	template<BufferPoolCreator C>
	class BufferPool : public hrs::non_copyable
	{
		void init(BoundedBufferSize &&_bounded_buffer_size) noexcept;

	public:
		BufferPool(Device *_parent_device) noexcept;
		~BufferPool();
		BufferPool(BufferPool &&bp) noexcept;
		BufferPool & operator=(BufferPool &&bp) noexcept;

		hrs::unexpected_result Recreate(vk::DeviceSize buffer_size);

		void Destroy() noexcept;
		bool IsCreated() const noexcept;
		bool IsEmpty() const noexcept;

		void Clear() noexcept;

		BoundedBufferSize & GetBoundedBufferSize() noexcept;
		const BoundedBufferSize & GetBoundedBufferSize() const noexcept;
		const hrs::sized_free_block_chain<vk::DeviceSize> & GetFreeBlocks() const noexcept;

		void Release(const hrs::block<vk::DeviceSize> &blk);
		void ReleaseAll();

		std::optional<hrs::block<vk::DeviceSize>> Acquire(vk::DeviceSize size, vk::DeviceSize alignment);

	private:
		Device *parent_device;
		BoundedBufferSize bounded_buffer_size;
		hrs::sized_free_block_chain<vk::DeviceSize> free_blocks;
	};

	template<BufferPoolCreator C>
	void BufferPool<C>::init(BoundedBufferSize &&_bounded_buffer_size) noexcept
	{
		bounded_buffer_size = std::move(_bounded_buffer_size);
		free_blocks.clear(bounded_buffer_size.size, bounded_buffer_size.bounded_buffer.block_bind_pool.block.offset);
	}

	template<BufferPoolCreator C>
	BufferPool<C>::BufferPool(Device *_parent_device) noexcept
		: parent_device(_parent_device)
	{
		hrs::assert_true_debug(_parent_device, "Parent device pointer points to null!");
		hrs::assert_true_debug(_parent_device->GetHandle(), "Parent device isn't created yet!");
	}

	template<BufferPoolCreator C>
	BufferPool<C>::~BufferPool()
	{
		Destroy();
	}

	template<BufferPoolCreator C>
	BufferPool<C>::BufferPool(BufferPool &&bp) noexcept
		: parent_device(bp.parent_device),
		  bounded_buffer_size(std::move(bp.bounded_buffer_size)),
		  free_blocks(std::move(bp.free_blocks)) {}

	template<BufferPoolCreator C>
	BufferPool<C> & BufferPool<C>::operator=(BufferPool &&bp) noexcept
	{
		Destroy();

		parent_device = bp.parent_device;
		bounded_buffer_size = std::move(bp.bounded_buffer_size);
		free_blocks = std::move(bp.free_blocks);

		return *this;
	}

	template<BufferPoolCreator C>
	hrs::unexpected_result BufferPool<C>::Recreate(vk::DeviceSize buffer_size)
	{
		if(buffer_size == 0)
			return {};

		const vk::BufferCreateInfo info({},
										buffer_size,
										C::GetBufferUsage(),
										vk::SharingMode::eExclusive);

		auto allocation_exp = AllocateFromMany(*parent_device->GetAllocator(),
											   C::GetAllocationVariants(),
											   info,
											   C::GetNewPoolSizeCalculator());

		if(allocation_exp)
		{
			vk::DeviceSize buffer_memory_offset = allocation_exp.value().block_bind_pool.block.offset;
			init({std::move(allocation_exp.value()), buffer_size});
			return {};
		}

		return allocation_exp.error().visit([]<typename E>(E e) -> hrs::unexpected_result
		{
			if constexpr(std::same_as<E, vk::Result>)
				return UnexpectedVkResult(e);
			else if constexpr(std::same_as<E, AllocatorResult>)
				return UnexpectedAllocatorResult(e);
			else//MemoryPoolResult
				return UnexpectedMemoryPoolResult(e);
		});
	}

	template<BufferPoolCreator C>
	void BufferPool<C>::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		parent_device->GetAllocator()->Release(bounded_buffer_size.bounded_buffer, MemoryPoolOnEmptyPolicy::Free);
		bounded_buffer_size = {};
		free_blocks.clear();
	}

	template<BufferPoolCreator C>
	bool BufferPool<C>::IsCreated() const noexcept
	{
		return bounded_buffer_size.bounded_buffer.IsCreated();
	}

	template<BufferPoolCreator C>
	bool BufferPool<C>::IsEmpty() const noexcept
	{
		return free_blocks.is_empty();
	}

	template<BufferPoolCreator C>
	void BufferPool<C>::Clear() noexcept
	{
		free_blocks.clear(bounded_buffer_size.size, bounded_buffer_size.bounded_buffer.block_bind_pool.block.offset);
	}

	template<BufferPoolCreator C>
	BoundedBufferSize & BufferPool<C>::GetBoundedBufferSize() noexcept
	{
		return bounded_buffer_size;
	}

	template<BufferPoolCreator C>
	const BoundedBufferSize & BufferPool<C>::GetBoundedBufferSize() const noexcept
	{
		return bounded_buffer_size;
	}

	template<BufferPoolCreator C>
	const hrs::sized_free_block_chain<vk::DeviceSize> & BufferPool<C>::GetFreeBlocks() const noexcept
	{
		return free_blocks;
	}

	template<BufferPoolCreator C>
	void BufferPool<C>::Release(const hrs::block<vk::DeviceSize> &blk)
	{
		hrs::assert_true_debug(blk.size % C::GetBlockSizePower() == 0,
							   "Block size is not a power of creator's block size!");

		hrs::assert_true_debug((blk.offset + free_blocks.get_outer_offset()) % C::GetBlockSizePower() == 0,
							   "Block offset is not a power of creator's block size!");

		free_blocks.release(blk);
	}

	template<BufferPoolCreator C>
	void BufferPool<C>::ReleaseAll()
	{
		Clear();
	}

	template<BufferPoolCreator C>
	std::optional<hrs::block<vk::DeviceSize>> BufferPool<C>::Acquire(vk::DeviceSize size, vk::DeviceSize alignment)
	{
		return free_blocks.acquire(size, alignment);
	}
};
