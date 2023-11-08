#pragma once

#include <concepts>
#include <list>
#include <vector>
#include <optional>
#include "block.hpp"
#include "debug.hpp"

namespace hrs
{
	template<std::unsigned_integral T>
	class free_block_allocator
	{
	public:
		using inner_value_type = T;
		using iterator = std::list<block<T>>::iterator;

		free_block_allocator(T _block_size, T blocks_count)
		{
			assert_true_debug(is_power_of_two(_block_size), "Block size must be power of 2!");
			memory_size = _block_size * blocks_count;
			block_size = _block_size;
			if(memory_size != 0)
				free_blocks.push_back(block<T>{memory_size, 0});
		}

		free_block_allocator(const free_block_allocator &l) :
			free_blocks(l.free_blocks), memory_size(l.memory_size), block_size(l.block_size) {}

		free_block_allocator(free_block_allocator &&l) noexcept :
			free_blocks(std::move(l.free_blocks)), memory_size(l.memory_size), block_size(l.block_size)
		{
			l.memory_size = 0;
		}

		auto operator=(const free_block_allocator &l) -> free_block_allocator &
		{
			free_blocks = l.free_blocks;
			memory_size = l.memory_size;
			block_size = l.block_size;
			return *this;
		}

		auto operator=(free_block_allocator &&l) noexcept -> free_block_allocator &
		{
			free_blocks = std::move(l.free_blocks);
			memory_size = l.memory_size;
			block_size = l.block_size;
			l.memory_size = 0;
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

		auto get_free_blocks() const -> std::list<block<T>>
		{
			return free_blocks;
		}

		auto get_free_blocks_count() const -> T
		{
			T blocks_count = 0;
			for(auto &free_blk : free_blocks)
				blocks_count += free_blk.size / block_size;

			return blocks_count;
		}

		auto clear(T new_block_size = 1, T new_blocks_count = {}) -> void
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

		auto acquire_blocks_no_append_hint(const T acq_block_size, T count) noexcept -> std::optional<iterator>
		{
			assert_true_debug(is_multiple_of(acq_block_size, block_size), "Requested block size must be multiple of block_size!");
			T common_size = acq_block_size * count;
			for(auto free_block_it = free_blocks.begin(); free_block_it != free_blocks.end(); free_block_it++)
			{
				if(free_block_it->size >= common_size)
					return free_block_it;
			}

			return {};
		}

		auto acquire_blocks(const T acq_block_size, T count) -> std::pair<T, block<T>>//appended blocks count and block
		{
			if(count == 0)
				return {};

			assert_true_debug(is_multiple_of(acq_block_size, block_size), "Requested block size must be multiple of block_size!");
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

			//HERE SOMETHING WRONG WITH free_blocks.size()??????????????
			if(!free_blocks.empty() && free_blocks.back().offset + free_blocks.size() == size())
			{
				T size_to_append = common_size - (size() - free_blocks.back().offset);
				block<T> blk{size_to_append, free_blocks.back().offset};
				memory_size += size_to_append;
				free_blocks.erase(std::prev(free_blocks.end()));
				return {size_to_append / block_size, blk};
			}
			else
			{
				//push (block_size * count_ / block_size
				block<T> blk{common_size, size()};
				memory_size += common_size;
				return {common_size / block_size, blk};
			}
		}

		auto release_block(const block<T> &blk) -> void
		{
			assert_true_debug(is_multiple_of(blk.size, block_size), "Requested block size must be multiple of block_size!");
			assert_true_debug(is_multiple_of(blk.offset + blk.size, block_size), "Requested block offset must be multiple of block_size!");

			if(free_blocks.empty())
			{
				assert_true_debug(blk.offset + blk.size <= size(), "Requested block space is bigger than entire space of allocator!");
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

		auto acquire_blocks_hint(const T acq_block_size, T count, iterator &hint_it) noexcept -> block<T>
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

			assert_true_debug(is_multiple_of(acq_block_size, block_size), "Requested block size must be multiple of block_size!");
			T common_size = acq_block_size * count;
			assert_true_debug(hint_it->size >= common_size, "Requested block size must be multiple of block_size!");

			block<T> blk{common_size, hint_it->offset};
			hint_it->size -= common_size;
			hint_it->offset += common_size;
			if(hint_it->size == 0)
				free_blocks.erase(hint_it);
			return {0, blk};
		}

	private:
		std::list<block<T>> free_blocks;
		T memory_size;
		T block_size;
	};
};
