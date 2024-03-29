#pragma once

#include "free_block_chain_base.hpp"

namespace hrs
{
	template<std::unsigned_integral T>
	class unsized_free_block_chain : public free_block_chain_base<T>
	{
	public:
		unsized_free_block_chain(T _size = 0, T _outer_offset = 0)
			: free_block_chain_base<T>(_size, _outer_offset) {}

		unsized_free_block_chain(const unsized_free_block_chain &) = default;
		unsized_free_block_chain(unsized_free_block_chain &&chain) = default;
		unsized_free_block_chain & operator=(const unsized_free_block_chain &) = default;
		unsized_free_block_chain & operator=(unsized_free_block_chain &&chain) = default;

		//block and added space
		std::pair<block<T>, T> acquire(const mem_req<T> &req) noexcept
		{
			if(req.size == 0)
				return block<T>{0, 0};

			auto blk_opt = this->acquire_from_existed(req);
			if(blk_opt)
				return {blk_opt.value(), 0};

			if(this->blocks.empty())//push back
				return acquire_from_back_no_blocks(req.size, req.alignment);
			else if(this->blocks.back().offset + this->blocks.back() == this->size)
			{
				T corrected_blk_offset = this->blocks.back().offset + this->outer_offset;
				T old_size = this->size;
				if(hrs::is_multiple_of(corrected_blk_offset, req.alignment))
				{
					T offset = this->blocks.back().offset;
					this->size += req.size - this->blocks.back().size;
					this->blocks.erase(std::prev(this->blocks.end()));
					return {block<T>(req.size, offset), this->size - old_size};
				}
				else
				{
					T new_offset = hrs::round_up_size_to_alignment(corrected_blk_offset, req.alignment) -
								   this->outer_offset;
					if(new_offset >= this->blocks.back().offset + this->blocks.back().size)
					{
						T remain_size = new_offset - (this->blocks.back().offset + this->blocks.back().size);
						this->size += req.size + remain_size;
						this->blocks.back().size = new_offset - this->blocks.back().offset;
						if(this->blocks.back().size == 0)
							this->blocks.erase(std::prev(this->blocks.end()));
						return {block<T>(req.size, new_offset), this->size - old_size};
					}
					else
					{
						T remain_size = (this->blocks.back().offset + this->blocks.back().size) - new_offset;
						this->blocks.back().size = new_offset - this->blocks.back().offset;
						this->size += req.size - remain_size;
						return {block<T>(req.size, new_offset), this->size - old_size};
					}
				}
			}
			else
				return acquire_from_back_no_blocks(req);
		}

		void increase_size(T delta)
		{
			if(this->blocks.empty())
				this->blocks.push_back(block<T>(delta, this->size));
			else if(this->is_back_block_adjacent_to_edge())
				this->blocks.back().size += delta;
			else
				this->blocks.push_back(block<T>(delta, this->size));

			this->size += delta;
		}

	private:
		std::pair<block<T>, T> acquire_from_back_no_blocks(const mem_req<T> &req)
		{
			T corrected_blk_offset = this->size + this->outer_offset;
			if(hrs::is_multiple_of(corrected_blk_offset, req.alignment))
			{
				//block_size = 4
				//          s
				//***********0000
				//  !  !  !  !  !
				T old_size = this->size;
				this->size += req.size;
				return {block<T>(req.size, old_size), req.size};
			}
			else
			{
				T old_size = this->size;
				T new_size =
					hrs::round_up_size_to_alignment(corrected_blk_offset, req.alignment) - this->outer_offset;
				this->size = new_size + req.size;
				return {block<T>(req.size, new_size), this->size - old_size};
			}
		}

	};
};

