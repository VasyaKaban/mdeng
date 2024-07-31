#pragma once

#pragma once

#include "mem_req.hpp"
#include "debug.hpp"
#include <list>
#include <utility>

namespace hrs
{
	template<std::unsigned_integral T>
	class free_block_chain_base
	{
	public:
		free_block_chain_base(T _size = 0, T _outer_offset = 0)
			: size(_size),
			  outer_offset(_outer_offset)
		{
			if(size != 0)
				blocks.push_back(block<T>(size, 0));
		}

		free_block_chain_base(const free_block_chain_base &) = default;

		free_block_chain_base(free_block_chain_base &&chain) noexcept
			: blocks(std::move(chain.blocks)),
			  size(std::exchange(chain.size, 0)),
			  outer_offset(std::exchange(chain.outer_offset, 0)) {}

		free_block_chain_base & operator=(const free_block_chain_base &) = default;

		free_block_chain_base & operator=(free_block_chain_base &&chain) noexcept
		{
			blocks = std::move(chain.blocks);
			size = std::exchange(chain.size, 0);
			outer_offset = std::exchange(chain.outer_offset, 0);

			return *this;
		}

		bool is_empty() const noexcept
		{
			if(blocks.size() == 1)
				return (blocks.front().offset == 0 && blocks.front().size == size);

			return false;
		}

		bool is_full() const noexcept
		{
			return blocks.empty();
		}

		T get_size() const noexcept
		{
			return size;
		}

		T get_outer_offset() const noexcept
		{
			return outer_offset;
		}

		void clear(T _size = 0, T _outer_offset = 0)
		{
			if(_size != 0 && !blocks.empty())
			{
				blocks.erase(std::next(blocks.begin()), blocks.end());
				(*blocks.begin()) = block<T>{_size, 0};
			}
			else if(_size != 0)
			{
				blocks.push_back(block<T>(_size, 0));
			}
			else
				blocks.clear();

			size = _size;
			outer_offset = _outer_offset;
		}

		void release(const block<T> &blk)
		{
			hrs::assert_true_debug(blk.offset + blk.size <= size,
								   "Block range is out of chain bounds!");

			if(blk.size == 0)
				return;

			if(blocks.empty())
				blocks.push_back(blk);
			const auto end_it = blocks.end();
			auto prev_it = blocks.end();
			auto post_it = blocks.end();
			for(auto it = blocks.begin(); it != blocks.end(); it++)
			{
				hrs::assert_true_debug(!are_blocks_overlapping(*it, blk),
									   "Requested release block overlaps free block!");

				if(it->offset > blk.offset + blk.size)
				{
					post_it = it;
					break;
				}

				prev_it = it;
			}

			if(prev_it != end_it && post_it == end_it)
			{
				if(prev_it->offset + prev_it->size == blk.offset)
					prev_it->size += blk.size;
				else
					blocks.push_back(blk);

			}
			else if(prev_it == end_it && post_it != end_it)
			{
				if(post_it->offset == blk.offset + blk.size)
				{
					post_it->offset = blk.offset;
					post_it->size += blk.size;
				}
				else
					blocks.push_front(blk);
			}
			else//prev_it != end_it && post_it != end_it
			{
				if(prev_it->offset + prev_it->size == blk.offset)//prev_it on edge
				{
					prev_it->size += blk.size;
					if(prev_it->offset + prev_it->size == post_it->offset)//post_it on edge
					{
						prev_it->size += post_it->size;
						blocks.erase(post_it);
					}
				}
				else
				{
					if(blk.offset + blk.size == post_it->offset)//post_it on edge
					{
						post_it->offset = blk.offset;
						post_it->size += blk.size;
					}
					else//blk is between prev_it and post_it
						blocks.insert(post_it, blk);
				}
			}
		}

		std::list<block<T>>::iterator begin() noexcept
		{
			return blocks.begin();
		}

		std::list<block<T>>::iterator end() noexcept
		{
			return blocks.end();
		}

		std::list<block<T>>::const_iterator begin() const noexcept
		{
			return blocks.cbegin();
		}

		std::list<block<T>>::const_iterator end() const noexcept
		{
			return blocks.cend();
		}

	protected:

		constexpr static bool are_blocks_overlapping(const block<T> &blk1, const block<T> &blk2) noexcept
		{
			block<T> prev_blk = blk1;
			block<T> post_blk = blk2;
			if(prev_blk.offset > post_blk.offset)
			{
				prev_blk = blk2;
				post_blk = blk1;
			}

				   //o********s
				   //     o************s
			return !(prev_blk.offset + prev_blk.size <= post_blk.offset);
			//return (post_blk.offset >= prev_blk.offset && post_blk.offset <= (prev_blk.offset + prev_blk.size));
		}

		std::optional<block<T>> acquire_from_existed(const mem_req<T> &req)
		{
			hrs::assert_true_debug(req.is_alignment_power_of_two(), "Alignment is not power of two!");

			if(blocks.empty())
				return {};

			for(auto it = blocks.begin(); it != blocks.end(); it++)
			{
				if(it->size < req.size)
					continue;

				if(it->size >= req.size)
				{
					T corrected_it_blk_offset = it->offset + outer_offset;
					if(hrs::is_multiple_of(corrected_it_blk_offset, req.alignment))
					{
						const block<T> out_blk(req.size, it->offset);
						handle_block_it(it, req.size);
						return out_blk;
					}
					else
					{
						auto split_opt = split_block(*it, req.alignment);
						if(!split_opt)
							continue;

						auto [remainder_blk, acquire_blk] = split_opt.value();
						if(acquire_blk.size < req.size)
							continue;

						const block<T> out_blk(req.size, acquire_blk.offset);
						handle_block_it(it, req.size, remainder_blk, acquire_blk);
						return out_blk;
					}
				}
			}

			return {};
		}

		constexpr std::optional<std::pair<block<T>, block<T>>>
		split_block(const block<T> &blk, T block_alignment) const noexcept
		{
			T corrected_block_offset = blk.offset + outer_offset;
			T aligned_corrected_block_offset = hrs::round_up_size_to_alignment(corrected_block_offset, block_alignment);
			if(aligned_corrected_block_offset == corrected_block_offset)
				return std::pair{block<T>(0, 0), blk};
			else if(aligned_corrected_block_offset > corrected_block_offset + blk.size)
				return {};
			else
			{
				//b************
				//     n*******

				T new_block_offset = aligned_corrected_block_offset - outer_offset;
				return std::pair{block<T>(new_block_offset - blk.offset, blk.offset),
								 block<T>(blk.size - (new_block_offset - blk.offset), new_block_offset)};
			}
		}

		void handle_block_it(std::list<block<T>>::iterator it, T block_size) noexcept
		{
			if(it->size == block_size)
				blocks.erase(it);
			else
			{
				it->size -= block_size;
				it->offset += block_size;
			}
		}

		void handle_block_it(std::list<block<T>>::iterator it,
							 T block_size,
							 const block<T> &remainder_blk,
							 block<T> acquire_blk) noexcept
		{

			(*it) = remainder_blk;
			if(acquire_blk.size != block_size)
			{
				acquire_blk.size -= block_size;
				acquire_blk.offset += block_size;

				blocks.insert(std::next(it), acquire_blk);
			}
		}

		bool is_back_block_adjacent_to_edge() const noexcept
		{
			return blocks.back().offset + blocks.back().size == size;
		}


	protected:
		std::list<block<T>> blocks;
		T size;
		T outer_offset;
	};
};


