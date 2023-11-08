#pragma once

#include "block.hpp"

namespace hrs
{
	template<std::unsigned_integral T>
	class linear_allocator
	{
	public:
		constexpr linear_allocator(T _block_size, T blocks_count) noexcept
		{
			assert_true_debug(is_power_of_two(_block_size), "Block size must be power of 2!");
			block_size = _block_size;
			memory_size = block_size * blocks_count;
			target_offset = 0;
		}

		constexpr linear_allocator(const linear_allocator &la) noexcept :
			memory_size(la.memory_size), target_offset(la.taregt_offset), block_size(la.block_size)
		{}

		constexpr linear_allocator(linear_allocator &la) noexcept :
			memory_size(la.memory_size), target_offset(la.target_offset), block_size(la.block_size)
		{
			la.block_size = 1;
			la.memory_size = 0;
			la.target_offset = 0;
		}

		constexpr auto operator=(const linear_allocator &la) noexcept -> linear_allocator &
		{
			target_offset = la.target_offset;
			memory_size = la.memory_size;
			block_size = la.block_size;
			return *this;
		}

		constexpr auto operator=(linear_allocator &&la) noexcept -> linear_allocator &
		{
			target_offset = la.target_offset;
			memory_size = la.memory_size;
			block_size = la.block_size;
			la.target_offset = 0;
			la.memory_size = 0;
			la.block_size = 1;
			return *this;
		}

		constexpr auto get_block_size() noexcept -> T
		{
			return block_size;
		}

		constexpr explicit operator bool() const noexcept
		{
			return memory_size != 0;
		}

		constexpr auto size() const noexcept -> T
		{
			return memory_size;
		}

		constexpr auto free_size() const noexcept -> T
		{
			return memory_size - target_offset;
		}

		constexpr auto get_offset() const noexcept -> T
		{
			return target_offset;
		}

		constexpr auto append_blocks(T added_blocks_count) noexcept -> void
		{
			memory_size += added_blocks_count * block_size;
		}

		constexpr auto get_free_blocks_count() const noexcept -> T
		{
			return (memory_size - target_offset) / block_size;
		}

		constexpr auto clear(T new_block_size = 1, T new_blocks_count = {}) noexcept -> void
		{
			assert_true_debug(is_power_of_two(new_block_size), "Block size must be power of 2!");
			block_size = new_block_size;
			memory_size = new_blocks_count * block_size;
			target_offset = 0;
		}

		constexpr auto acquire_blocks(const T acq_block_size, T count) noexcept -> std::pair<T, block<T>>
		{
			if(count == 0)
				return {};

			assert_true_debug(is_multiple_of(acq_block_size, block_size), "Requested block size must be multiple of block_size!");
			T common_size = acq_block_size * count;
			T target_free_size = free_size();
			if(target_free_size >= common_size)
			{
				block<T> blk(common_size, target_offset);
				target_offset += acq_block_size * count;
				return {0, blk};
			}
			else
			{
				block<T> blk(common_size, target_offset);
				T diff_size = common_size - target_free_size;
				memory_size += diff_size;
				target_offset = memory_size;
				return {diff_size / block_size, blk};
			}
		}

		constexpr auto release_block(T acq_block_size) noexcept -> void
		{
			assert_true_debug(is_multiple_of(acq_block_size, block_size), "Requested block size must be multiple of block_size!");
			assert_true_debug(target_offset >= acq_block_size, "Target block offset is smaller than requested to release size!");
			target_offset = target_offset - acq_block_size;
		}

		constexpr auto can_acquire_no_append(T acq_block_size) const noexcept -> bool
		{
			assert_true_debug(is_multiple_of(acq_block_size, block_size), "Requested block size must be multiple of block_size!");
			if(memory_size < acq_block_size)
				return false;

			return memory_size - acq_block_size >= target_offset;
		}

	private:
		T target_offset;
		T memory_size;
		T block_size;
	};
};
