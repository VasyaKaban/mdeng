/**
 * @file
 *
 * Represents the free_block_allocator class
 */

#pragma once

#include <concepts>
#include <list>
#include <vector>
#include <optional>
#include "block.hpp"
#include "debug.hpp"

namespace hrs
{
	/**
	 * @brief The free_block_allocator class
	 * @tparam T must satisfy unsigned_integral concept
	 *
	 * Allocator based on list of free blocks
	 */
	template<std::unsigned_integral T>
	class free_block_allocator
	{
	public:
		///same as T
		using inner_value_type = T;
		///the iterator type that is used inside the inner container
		using iterator = std::list<block<T>>::iterator;

		/**
		 * @brief free_block_allocator
		 * @param _block_size the size of block
		 * @param blocks_count a count of blocks with _block_size
		 *
		 * Creates new free_block_allocator with one block with zero offset and
		 * size equal to memory_size where memory_size is:
		 * @code
		 *	memory_size = _block_size * blocks_count;
		 * @endcode
		 */
		free_block_allocator(T _block_size, T blocks_count)
		{
			assert_true_debug(is_power_of_two(_block_size), "Block size must be power of 2!");
			memory_size = _block_size * blocks_count;
			block_size = _block_size;
			if(memory_size != 0)
				free_blocks.push_back(block<T>{memory_size, 0});
		}

		free_block_allocator(const free_block_allocator &l)
			: free_blocks(l.free_blocks), memory_size(l.memory_size), block_size(l.block_size) {}

		free_block_allocator(free_block_allocator &&l) noexcept
			: free_blocks(std::move(l.free_blocks)), memory_size(l.memory_size), block_size(l.block_size)
		{
			l.memory_size = 0;
		}

		free_block_allocator & operator=(const free_block_allocator &l)
		{
			free_blocks = l.free_blocks;
			memory_size = l.memory_size;
			block_size = l.block_size;
			return *this;
		}

		free_block_allocator & operator=(free_block_allocator &&l) noexcept
		{
			free_blocks = std::move(l.free_blocks);
			memory_size = l.memory_size;
			block_size = l.block_size;
			l.memory_size = 0;
			return *this;
		}

		/**
		 * @brief get_block_size
		 * @return a size of block that was previously passed to the constructor
		 */
		constexpr T get_block_size() const noexcept
		{
			return block_size;
		}

		/**
		 * @brief operator bool
		 *
		 * Checks if the size of memory isn't equal to zero
		 */
		constexpr explicit operator bool() const noexcept
		{
			return memory_size != 0;
		}

		/**
		 * @brief size
		 * @return size of memory
		 */
		constexpr T size() const noexcept
		{
			return memory_size;
		}

		/**
		 * @brief append_space
		 * @param added_blocks_count count of blocks to append
		 *
		 * Appends added_blocks_count * block_size bytes to the end of allocator
		 */
		auto append_space(T added_blocks_count) -> void
		{
			if(added_blocks_count == 0)
				return;

			if(free_blocks.empty())
				free_blocks.push_back(block<T>{added_blocks_count * block_size, size()});
			else
			{
				T edge = free_blocks.back().offset + free_blocks.back().size;
				if(edge == size())
				{
					//back block is on edge -> append size into edge block
					free_blocks.back().size += added_blocks_count * block_size;
				}
				else
					free_blocks.push_back(block<T>{added_blocks_count * block_size, size()});
			}

			memory_size += added_blocks_count * block_size;
		}

		/**
		 * @brief get_free_blocks
		 * @return list of all free blocks of this allocator
		 */
		std::list<block<T>> get_free_blocks() const
		{
			return free_blocks;
		}

		/**
		 * @brief get_free_blocks_count
		 * @return count of free blocks with size of 'block_size'
		 */
		T get_free_blocks_count() const noexcept
		{
			T blocks_count = 0;
			for(auto &free_blk : free_blocks)
				blocks_count += free_blk.size / block_size;

			return blocks_count;
		}

		/**
		 * @brief clear
		 * @param new_block_size new size of block
		 * @param new_blocks_count count of blocks with new_block_size size
		 *
		 * Clears list of free blocks and recreates allocator with new parameters
		 * @warning Aborts when new_block_size is not a multiple of target block size!
		 */
		void clear(T new_block_size = 1, T new_blocks_count = {})
		{
			assert_true_debug(is_power_of_two(new_block_size), "Block size must be power of 2!");
			if(new_blocks_count == 0)
				free_blocks.clear();
			else if(free_blocks.empty())
				free_blocks.push_back(block<T>{new_blocks_count * new_block_size, 0});
			else
			{
				free_blocks.erase(free_blocks.begin() + 1, free_blocks.end());
				free_blocks.front() = block<T>{new_blocks_count * new_block_size, 0};
			}

			memory_size = new_blocks_count * block_size;
			block_size = new_block_size;
		}

		/**
		 * @brief size_to_block_size
		 * @param size size that is queried to be rounded up to block size of allocator
		 * @return size that rounded up to alignment of the target block size
		 */
		T size_to_block_size(T size) const noexcept
		{
			return round_up_size_to_alignment(size, block_size);
		}

		/**
		 * @brief acquire_blocks_no_append_hint
		 * @param acq_block_size size of block to acquire
		 * @param count count of blocks with desired size
		 * @return iterator to free block that have enough space to place blocks
		 * @warning aborts when acq_block_size is not a multiple of target block size!
		 */
		std::optional<iterator> acquire_blocks_no_append_hint(T acq_block_size, T count) noexcept
		{
			assert_true_debug(is_multiple_of(acq_block_size, block_size),
							  "Requested block size must be multiple of block_size!");
			T common_size = acq_block_size * count;
			for(auto free_block_it = free_blocks.begin(); free_block_it != free_blocks.end(); free_block_it++)
			{
				if(free_block_it->size >= common_size)
					return free_block_it;
			}

			return {};
		}

		/**
		 * @brief acquire_blocks
		 * @param acq_block_size size of block to acquire
		 * @param count count of blocks with desired size
		 * @return pair that consists of appended blocks count and block itself
		 *
		 * If count is zero returns empty pair.
		 * Otherwise returns pair of zero appended blocks and acquired block if it finds free block that
		 * satisfies requested block space.
		 * If there are no free block with desired space then it appends a space that is not enough
		 * and returns a pair of appended blocks and acquired block
		 * @warning Aborts when acq_block_size is not a multiple of target block size!
		 */
		std::pair<T, block<T>> acquire_blocks(T acq_block_size, T count)//appended blocks count and block
		{
			if(count == 0)
				return {};

			assert_true_debug(is_multiple_of(acq_block_size, block_size),
							  "Requested block size must be multiple of block_size!");
			T common_size = acq_block_size * count;
			for(auto free_block_it = free_blocks.begin(); free_block_it != free_blocks.end(); free_block_it++)
			{
				if(free_block_it->size >= common_size)
				{
					block<T> blk{common_size, free_block_it->offset};
					free_block_it->size -= common_size;
					free_block_it->offset += common_size;
					if(free_block_it->size == 0)
						free_blocks.erase(free_block_it);
					return {0, blk};
				}
			}

			//can append
			if(!free_blocks.empty() && (free_blocks.back().offset + free_blocks.size() == size()))
			{
				//if we have back free-block that borders with edge
				T size_to_append = common_size - (size() - free_blocks.back().offset);
				block<T> blk{common_size, free_blocks.back().offset};
				memory_size += size_to_append;
				free_blocks.erase(std::prev(free_blocks.end()));
				return {size_to_append / block_size, blk};
			}
			else
			{
				//if we don't have border free-block
				block<T> blk{common_size, size()};
				memory_size += common_size;
				return {common_size / block_size, blk};
			}
		}

		/**
		 * @brief release_block
		 * @param blk block to release
		 *
		 * Releases a block
		 * @warning Aborts when block size and size + offset aren't multiple of target block size!
		 */
		void release_block(const block<T> &blk)
		{
			assert_true_debug(is_multiple_of(blk.size, block_size),
							  "Requested block size must be multiple of block_size!");
			assert_true_debug(is_multiple_of(blk.offset + blk.size, block_size),
							  "Requested block offset must be multiple of block_size!");

			if(free_blocks.empty())
			{
				assert_true_debug(blk.offset + blk.size <= size(),
								  "Requested block space is bigger than entire space of allocator!");
				free_blocks.push_back(blk);
				return;
			}

			iterator pre_blk = free_blocks.end();
			iterator post_blk = free_blocks.end();
			for(auto it = free_blocks.begin(); it != free_blocks.end(); it++)
			{
				if(it->offset >= blk.offset + blk.size)
					break;

				pre_blk = it;
			}

			if(pre_blk != free_blocks.end())
				post_blk = std::next(pre_blk);
			else
				post_blk = free_blocks.begin();

			iterator blk_it = free_blocks.end();
			if(pre_blk != free_blocks.end())
			{
				//insert after pre_blk
				if(pre_blk->offset + pre_blk->size == blk.offset)
				{
					pre_blk->size += blk.size;
					blk_it = pre_blk;
				}
				else
					blk_it = free_blocks.insert(std::next(pre_blk), blk);
			}
			else
				blk_it = free_blocks.insert(free_blocks.begin(), blk);

			if(post_blk != free_blocks.end())
			{
				//insert before post_blk
				if(blk_it->offset + blk_it->size == post_blk->offset)
				{
					blk_it->size += post_blk->size;
					free_blocks.erase(post_blk);
				}
			}
		}

		/**
		 * @brief acquire_blocks_hint
		 * @param acq_block_size size of block to acquire
		 * @param count desired count of blocks to acquire
		 * @param hint_it hint iterator to free block that has enough space
		 * @return acquired block
		 *
		 * Acquires requested block without looping over all free blocks by trying to
		 * acquire it from hint iterator
		 * @warning Aborts when acq_block_size is not a multiple of target block size and
		 * when hint iterator size is smaller than common size of requested blocks!
		 * @warning It checks hint iterator to be an iterator of target list of free blocks
		 * and aborts when it's false only on debug build!
		 */
		block<T> acquire_blocks_hint(T acq_block_size, T count, iterator &hint_it) noexcept
		{
			if(count == 0)
				return {};

			#ifndef NDEBUG
				bool hint_it_found = false;
				for(auto it = free_blocks.begin(); it != free_blocks.end(); it++)
					if(it == hint_it)
					{
						hint_it_found = true;
						break;
					}

				assert_true_debug(hint_it_found, "Hint iterator is not part of this list!");
			#endif

			assert_true_debug(is_multiple_of(acq_block_size, block_size),
								  "Requested block size must be multiple of block_size!");
			T common_size = acq_block_size * count;
			assert_true_debug(hint_it->size >= common_size,
							  "Hint iterator size smaller than requested block size!");

			block<T> blk{common_size, hint_it->offset};
			hint_it->size -= common_size;
			hint_it->offset += common_size;
			if(hint_it->size == 0)
				free_blocks.erase(hint_it);
			return blk;
		}

	private:
		std::list<block<T>> free_blocks;///container of free blocks
		T memory_size;///common size of all memory
		T block_size;///size and alignment of one block
	};
};
