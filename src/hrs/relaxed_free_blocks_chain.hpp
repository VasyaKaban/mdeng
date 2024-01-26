#pragma once

#include <list>
#include "block.hpp"
#include "debug.hpp"

namespace hrs
{
	template<std::unsigned_integral T>
	class relaxed_free_block_chain
	{
	public:

		class free_block_handle
		{
		public:
			friend class relaxed_free_block_chain;
			constexpr free_block_handle() = default;
			constexpr free_block_handle(std::list<block<T>>::iterator _handle)
				: handle(_handle) {}
			constexpr ~free_block_handle() = default;

		private:
			typename std::list<block<T>>::iterator handle;
		};

		relaxed_free_block_chain(std::size_t _size = 0)
		{
			size = _size;
			if(_size != 0)
				free_blocks.push_back(block<T>(size, 0));
		}

		~relaxed_free_block_chain() = default;

		relaxed_free_block_chain(const relaxed_free_block_chain &) = default;

		relaxed_free_block_chain(relaxed_free_block_chain &&chain) noexcept
			: size(chain.size), free_blocks(std::move(chain.free_blocks))
		{
			chain.size = 0;
		}

		relaxed_free_block_chain & operator=(const relaxed_free_block_chain &) = default;

		relaxed_free_block_chain & operator=(relaxed_free_block_chain &&chain) noexcept
		{
			size = chain.size;
			free_blocks = std::move(chain.free_blocks);
			chain.size = 0;
			return *this;
		}

		void clear(std::size_t _size = 0)
		{
			size = _size;
			if(_size != 0)
			{
				if(free_blocks.size() == 0)
					free_blocks.push_back(block<T>(size, 0));
				else
				{
					free_blocks.erase(std::next(free_blocks.begin(), 1), free_blocks.end());
					free_blocks.front().size = size;
					free_blocks.front().offset = 0;
				}
			}
		}

		std::pair<block<T>, std::size_t> acquire(T block_alignment, T block_size)
		{
			hrs::assert_true_debug(hrs::is_power_of_two(block_alignment),
								   "Alignment={} isn't power of two!", block_alignment);
			/*hrs::assert_true_debug(hrs::is_multiple_of(block_size, block_alignment),
								   "Size={} isn't power of alignment={}!",
								   block_size, block_alignment);*/

			if(block_size <= size)
			{
				for(auto free_block_it  = free_blocks.begin(); free_block_it != free_blocks.end(); free_block_it++)
				{
					if(free_block_it->size >= block_size)
					{
						auto split_blocks_opt = split_by_alignment_block(*free_block_it, block_alignment);
						if(split_blocks_opt)
						{
							auto &[remainder_blk, aligned_blk] = split_blocks_opt.value();
							if(aligned_blk.size >= block_size)
							{
								block<T> acquired_blk{.size = block_size, .offset = aligned_blk.offset};
								aligned_blk.size -= block_size;
								aligned_blk.offset += block_size;

								if(remainder_blk.size == 0 && aligned_blk.size == 0)
									free_blocks.erase(free_block_it);
								else if(remainder_blk.size != 0 && aligned_blk.size == 0)
									*free_block_it = remainder_blk;
								else if(remainder_blk.size == 0 && aligned_blk.size != 0)
									*free_block_it = aligned_blk;
								else
								{
									free_blocks.insert(free_block_it, remainder_blk);
									*free_block_it = aligned_blk;
								}

								return {acquired_blk, 0};
							}
						}
					}
				}
			}

			return create_back_block(block_alignment, block_size);
		}

		void release(const block<T> &blk)
		{
			hrs::assert_true_debug(blk.offset + blk.size <= size,
								   "Block isn't a part of this chain because offset({}) + size({}) > chain_size{}!",
								   blk.offset, blk.size, size);


			auto end_it = free_blocks.end();
			auto pre_it = end_it;
			auto post_it = end_it;

			for(auto it = free_blocks.begin(); it != free_blocks.end(); it++)
			{
				if(it->offset >= (blk.offset + blk.size))
				{
					post_it = it;
					break;
				}

				pre_it = it;
			}

			if(pre_it == end_it && post_it == end_it)
			{
				//no free blocks
				free_blocks.push_back(blk);
			}
			else if(pre_it != end_it && post_it == end_it)
			{
				if((pre_it->offset + pre_it->size) == blk.offset)//edge
					pre_it->size += blk.size;
				else
					free_blocks.insert(free_blocks.end(), blk);
			}
			else if(pre_it == end_it && post_it != end_it)
			{
				if((blk.offset + blk.size) == post_it->offset)//edge
					post_it->offset -= blk.size;
				else
					free_blocks.insert(free_blocks.begin(), blk);
			}
			else //if(pre_it != end_it && post_it != end_it)
			{
				if((pre_it->offset + pre_it->size) == blk.offset)//edge
				{
					pre_it->size += blk.size;
					if((pre_it->offset + pre_it->size) == post_it->offset)
					{
						pre_it->size += post_it->size;
						free_blocks.erase(post_it);
					}
				}
				else
				{
					if((blk.offset + blk.size) == post_it->offset)//edge
						post_it->offset -= blk.size;
				}
			}
		}

		bool is_empty() const noexcept
		{
			if(size == 0)
				return true;

			if(free_blocks.size() == 1 && free_blocks.front().offset == 0)
				return true;

			return false;
		}

		const std::list<block<T>> & get_free_blocks() const noexcept
		{
			return free_blocks;
		}

		T get_size() const noexcept
		{
			return size;
		}

		std::optional<free_block_handle> find_place_no_append(T block_size, T block_alignment) noexcept
		{
			hrs::assert_true_debug(hrs::is_power_of_two(block_alignment),
								   "Alignment={} isn't power of two!", block_alignment);
			/*hrs::assert_true_debug(hrs::is_multiple_of(block_size, block_alignment),
								   "Size={} isn't power of alignment={}!",
								   block_size, block_alignment);*/

			if(size < block_size)
				return {};

			for(auto it = free_blocks.begin(); it != free_blocks.end(); it++)
			{
				if(it->size >= block_size)
				{
					if(hrs::is_multiple_of(it->offset, block_alignment))
						return it;
					else
					{
						auto split_blk = split_by_alignment_block(*it, block_alignment);
						if(!split_blk)
							return {};

						auto &aligned_blk = split_blk.value().second;
						if(aligned_blk.size >= block_size)
							return it;
					}
				}
			}

			return {};
		}

		block<T> acquire_by_hint(free_block_handle hint, T block_size, T block_alignment)
		{
			hrs::assert_true_debug(hrs::is_power_of_two(block_alignment),
								   "Alignment={} isn't power of two!", block_alignment);
			/*hrs::assert_true_debug(hrs::is_multiple_of(block_size, block_alignment),
								   "Size={} isn't power of alignment={}!",
								   block_size, block_alignment);*/

		#ifndef NDEBUG
			bool found = false;
			for(auto it = free_blocks.begin(); it != free_blocks.end(); it++)
				if(it == hint.handle)
				{
					if((*it) == (*hint.handle))
					{
						found = true;
						break;
					}
				}

			hrs::assert_true_debug(found, "Handle is not a part of this chain!");
		#endif

			if(hrs::is_multiple_of(hint.handle->offset, block_alignment))
			{
				hrs::assert_true_debug(hint.handle->size >= block_size,
									   "Requested block size={} is bigger than hint size={}!",
									   block_size, hint.handle->size);

				block<T> out_blk(block_size, hint.handle->offset);
				hint.handle->size -= block_size;
				hint.handle->offset += block_size;
				if(hint.handle->size == 0)
					free_blocks.erase(hint.handle);

				return out_blk;
			}
			else
			{
				auto split_opt = split_by_alignment_block(*hint.handle, block_alignment);
				hrs::assert_true_debug(split_opt.has_value(),
									   "Handle size was changed after the the hint was requested!");
				auto &[remainder_blk, aligned_blk] = split_opt.value();
				hrs::assert_true_debug(aligned_blk.size >= block_size,
									   "Handle size was changed after the the hint was requested!");

				block<T> out_blk(block_size, aligned_blk.offset);
				if(remainder_blk.size == 0 && aligned_blk.size == 0)
					free_blocks.erase(hint.handle);
				else if(remainder_blk.size != 0 && aligned_blk.size == 0)
					*hint.handle = remainder_blk;
				else if(remainder_blk.size == 0 && aligned_blk.size != 0)
					*hint.handle = aligned_blk;
				else
				{
					free_blocks.insert(hint.handle, remainder_blk);
					*hint.handle = aligned_blk;
				}

				return out_blk;
			}
		}

	private:

		static std::optional<std::pair<block<T>, block<T>>>//remainder block and aligned block
		split_by_alignment_block(const block<T> &blk, T desired_alignment) noexcept
		{
			hrs::assert_true_debug(hrs::is_power_of_two(desired_alignment),
								   "Alignment={} isn't power of two!", desired_alignment);


			T rounded_offset = hrs::round_up_size_to_alignment(blk.offset, desired_alignment);
			if(rounded_offset == blk.offset)
				return std::pair{block<T>(0, blk.offset), blk};
			else if(rounded_offset > blk.size)
				return {};
			else
				return std::pair{block<T>(rounded_offset - blk.offset, blk.offset),
								 block<T>(blk.size - (rounded_offset - blk.offset), rounded_offset)};
		}

		std::pair<block<T>, T>
		create_back_block(T block_alignment, T block_size)
		{
			hrs::assert_true_debug(hrs::is_power_of_two(block_alignment),
								   "Alignment={} isn't power of two!", block_alignment);
			/*hrs::assert_true_debug(hrs::is_multiple_of(block_size, block_alignment),
								   "Size={} isn't power of alignment={}!",
								   block_size, block_alignment);*/

			if(!free_blocks.empty())
			{
				auto &back_blk = free_blocks.back();
				if(back_blk.offset + back_blk.size == size)//on edge
				{
					T new_offset = hrs::round_up_size_to_alignment(back_blk.offset, block_alignment);
					T new_size = new_offset + block_size;
					T delta_offset = new_offset - back_blk.offset;
					if(delta_offset == 0)
					{
						if(back_blk.size >= block_size)
						{
							//place here
							back_blk.size -= block_size;
							if(back_blk.size == 0)
								free_blocks.erase(std::prev(free_blocks.end()));

							return {block<T>{.size = block_size, .offset = new_offset}, 0};
						}
						else
						{
							//place here + allocate extra space
							free_blocks.erase(std::prev(free_blocks.end()));
							T appended_space = block_size - back_blk.size;
							size = new_size;
							return {block<T>{.size = block_size, .offset = new_offset}, appended_space};
						}
					}
					else
					{
						if((back_blk.size - delta_offset) >= block_size)
						{
							//place after new_offset + push remainder block
							back_blk.size -= (block_size + delta_offset);
							back_blk.offset = new_offset + block_size;
							if(back_blk.size == 0)
								back_blk.size = delta_offset;
							else
								free_blocks.insert(std::prev(free_blocks.end()),
												   block<T>{.size = delta_offset, .offset = new_offset - delta_offset});

							return {block<T>{.size = block_size, .offset = new_offset}, 0};
						}
						else
						{
							//push remainder block + allocate extra space
							back_blk.size = delta_offset;
							T appended_space = block_size - (back_blk.size - delta_offset);
							size = new_size;
							return {block<T>{.size = block_size, .offset = new_offset}, appended_space};
						}
					}
				}
			}

			T aligned_size = hrs::round_up_size_to_alignment(size, block_alignment);
			T delta_size = aligned_size - size;
			T appended_size = delta_size + block_size;

			if(delta_size != 0)
				free_blocks.push_back(block<T>{.size = delta_size, .offset = size});

			size = appended_size;
			return {block<T>{.size = block_size, .offset = aligned_size}, appended_size};
		}

		std::list<block<T>> free_blocks;
		T size;
	};
};
