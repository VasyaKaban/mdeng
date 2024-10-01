#include "BoundedBufferSizeFillness.h"

namespace FireLand
{
    BoundedBufferSizeFillness::BoundedBufferSizeFillness(BoundedBufferSize&& bbs,
                                                         VkDeviceSize _fillness) noexcept
        : BoundedBufferSize(std::move(bbs)),
          fillness(_fillness)
    {
        hrs::assert_true_debug(fillness <= this->size,
                               "Fillness = {} must be less than or equal to size = {}!",
                               fillness,
                               this->size);
    }

    BoundedBufferSizeFillness::BoundedBufferSizeFillness(BoundedBufferSizeFillness&& bbsf) noexcept
        : BoundedBufferSize(std::move(bbsf)),
          fillness(std::exchange(bbsf.fillness, 0))
    {}

    BoundedBufferSizeFillness&
    BoundedBufferSizeFillness::operator=(BoundedBufferSizeFillness&& bbsf) noexcept
    {
        BoundedBufferSize::operator=(std::move(bbsf));
        fillness = std::exchange(bbsf.fillness, 0);

        return *this;
    }

    bool BoundedBufferSizeFillness::IsFull() const noexcept
    {
        return fillness == size;
    }

    std::optional<VkDeviceSize>
    BoundedBufferSizeFillness::Append(const hrs::mem_req<VkDeviceSize>& req) noexcept
    {
        hrs::assert_true_debug(hrs::is_power_of_two(req.alignment),
                               "Requirement alignment = {} is not a power of two!",
                               req.alignment);
        hrs::assert_true_debug(hrs::is_multiple_of(req.size, req.alignment),
                               "Requirement size = {} is not multiple of alignment = {}!",
                               req.size,
                               req.alignment);

        if(req.size > size)
            return {};

        if(hrs::is_multiple_of(fillness, req.alignment))
        {
            if(req.size + fillness <= size)
            {
                VkDeviceSize tmp_fillness = fillness;
                fillness += req.size;
                return tmp_fillness;
            }

            return {};
        }
        else
        {
            VkDeviceSize new_fillness = hrs::round_up_size_to_alignment(fillness, req.alignment);
            if(new_fillness > size)
                return {};

            if(req.size + new_fillness <= size)
            {
                VkDeviceSize tmp_fillness = new_fillness;
                fillness = new_fillness + req.size;
                return tmp_fillness;
            }

            return {};
        }
    }
};
