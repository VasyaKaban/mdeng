#pragma once

#include "free_block_chain_base.hpp"

namespace hrs
{
    template<std::unsigned_integral T>
    class sized_free_block_chain : public free_block_chain_base<T>
    {
    public:
        sized_free_block_chain(T _size = 0, T _outer_offset = 0)
            : free_block_chain_base<T>(_size, _outer_offset)
        {}

        sized_free_block_chain(const sized_free_block_chain&) = default;
        sized_free_block_chain(sized_free_block_chain&& chain) = default;
        sized_free_block_chain& operator=(const sized_free_block_chain&) = default;
        sized_free_block_chain& operator=(sized_free_block_chain&& chain) = default;

        std::optional<block<T>> acquire(T block_size, T block_alignment) noexcept
        {
            return this->acquire_from_existed(mem_req<T>(block_size, block_alignment));
        }

        bool is_hint_valid(std::list<block<T>>::const_iterator hint_it) const noexcept
        {
            return hrs::is_iterator_part_of_range(this->blocks, hint_it);
        }

        bool is_block_can_be_placed(std::list<block<T>>::const_iterator hint_it,
                                    T block_size,
                                    T block_alignment) const noexcept
        {
            hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(this->blocks, hint_it),
                                   "Passed iterator hint is not part of this chain!");

            if(hint_it->size < block_size)
                return false;

            T corrected_it_blk_offset = hint_it->offset + this->outer_offset;
            if(hrs::is_multiple_of(corrected_it_blk_offset, block_alignment))
                return true;
            else
            {
                auto split_opt = this->split_block(*hint_it, block_alignment);
                if(!split_opt)
                    return false;

                auto [remainder_blk, acquire_blk] = split_opt.value();
                if(acquire_blk.size < block_size)
                    return false;

                return true;
            }
        }

        hrs::block<T> acquire_by_hint(std::list<block<T>>::iterator hint_it,
                                      const mem_req<T>& req) noexcept
        {
            hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(this->blocks, hint_it),
                                   "Passed iterator hint is not part of this chain!");

            hrs::assert_true_debug(is_block_can_be_placed(hint_it, req.size, req.alignment),
                                   "Requested block cannot be placed within hint iterator block!");

            T corrected_it_blk_offset = hint_it->offset + this->outer_offset;
            if(hrs::is_multiple_of(corrected_it_blk_offset, req.alignment))
            {
                const block<T> out_blk(req.size, hint_it->offset);
                this->handle_block_it(hint_it, req.size);
                return out_blk;
            }
            else
            {
                auto [remainder_blk, acquire_blk] =
                    this->split_block(*hint_it, req.alignment).value();

                const block<T> out_blk(req.size, acquire_blk.offset);
                this->handle_block_it(hint_it, req.size, remainder_blk, acquire_blk);
                return out_blk;
            }
        }
    };
};
