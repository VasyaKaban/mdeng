#include "MemoryType.h"
#include "../Context/DeviceLoader.h"
#include "MemoryPool.h"

namespace FireLand
{
    VkDeviceSize MemoryType::DefaultNewPoolSizeCalculator(bool initial,
                                                          VkDeviceSize previous_size,
                                                          VkDeviceSize requested_size,
                                                          const MemoryType& mem_type) noexcept
    {
        constexpr static VkDeviceSize divisor = 32;
        if(initial) //first call
        {
            const VkMemoryHeap& mem_heap = mem_type.heap;
            VkDeviceSize allocation_size = mem_heap.size / divisor;
            if(requested_size > allocation_size)
                allocation_size = (requested_size / allocation_size) * (allocation_size + 1);

            if(allocation_size > mem_heap.size || allocation_size < requested_size)
                return 0;

            return allocation_size;
        }
        else
        {
            previous_size = (previous_size >> 2) + (previous_size >> 4);
            if(previous_size < requested_size)
            {
                if(requested_size % divisor == 0)
                    return requested_size;
                else if(requested_size < divisor)
                    return 0;
                else
                    return (requested_size / divisor) * (divisor + 1);
                //do not think about allocations alignment(like power of two or smthing)
            }

            return previous_size;
        }
    }

    MemoryType::MemoryType(VkMemoryHeap _heap,
                           VkMemoryPropertyFlags _memory_property_flags,
                           std::uint32_t _index,
                           VkDeviceSize _buffer_image_granularity) noexcept
        : heap(_heap),
          memory_property_flags(_memory_property_flags),
          index(_index),
          buffer_image_granularity(_buffer_image_granularity)
    {
        hrs::assert_true_debug(hrs::is_power_of_two(_buffer_image_granularity),
                               "Buffer image granularity must be power of two!");
    }

    MemoryType::MemoryType(MemoryType&& mem_type) noexcept
        : heap(mem_type.heap),
          memory_property_flags(mem_type.memory_property_flags),
          index(mem_type.index),
          buffer_image_granularity(mem_type.buffer_image_granularity),
          lists(std::move(mem_type.lists))
    {}

    void MemoryType::Destroy(VkDevice device,
                             const DeviceLoader& dl,
                             const VkAllocationCallbacks* alc) noexcept
    {
        lists.Clear(device, dl, alc);
    }

    bool MemoryType::IsSatisfy(MemoryTypeSatisfyOp satisfy,
                               VkMemoryPropertyFlags props) const noexcept
    {
        if(satisfy == MemoryTypeSatisfyOp::Any)
            return (!props || ((memory_property_flags & props) == props));
        else
            return (!props || memory_property_flags == props);
    }

    bool MemoryType::IsSatisfyIndex(std::uint32_t memory_type_bits) const noexcept
    {
        return (memory_type_bits & GetMemoryTypeIndexMask()) == GetMemoryTypeIndexMask();
    }

    bool MemoryType::IsMappable() const noexcept
    {
        return static_cast<bool>(memory_property_flags &
                                 VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    }

    bool MemoryType::IsEmpty() const noexcept
    {
        return lists.IsEmpty();
    }

    std::uint32_t MemoryType::GetMemoryTypeIndex() const noexcept
    {
        return index;
    }

    std::uint32_t MemoryType::GetMemoryTypeIndexMask() const noexcept
    {
        return (1 << index);
    }

    hrs::expected<MemoryTypeAcquireResult, hrs::error>
    MemoryType::Allocate(ResourceType res_type,
                         const VkMemoryRequirements& req,
                         hrs::flags<AllocationFlags> flags,
                         VkDevice device,
                         const DeviceLoader& dl,
                         const VkAllocationCallbacks* alc,
                         const std::function<NewPoolSizeCalculator>& calc)
    {
        hrs::assert_true_debug(IsSatisfyIndex(req.memoryTypeBits),
                               "Memory type doesn't satisfy the memory requirements!");
        hrs::assert_true_debug(calc != nullptr,
                               "New pool size calculator function is null pointer!");
        hrs::assert_true_debug(hrs::is_power_of_two(req.alignment),
                               "Alignment is not power of two!");
        hrs::assert_true_debug((flags & AllocationFlags::MapMemory ? IsMappable() : true),
                               "Memory type is considered not to be mapped!");
        hrs::assert_true_debug((flags & AllocationFlags::AllocateSeparatePool &
                                AllocationFlags::CreateOnExistedPools) ?
                                   false :
                                   true,
                               "Conflicting flags: AllocateSeparatePool and "
                               "CreateOnExistedPools!");

        if(!(flags & AllocationFlags::AllocateSeparatePool))
        {
            //try acquire
            auto acq_exp = TryAcquire(res_type, req, flags, device, dl, alc, calc);
            if(acq_exp)
                return *acq_exp;
        }

        if(!(flags & AllocationFlags::CreateOnExistedPools))
        {
            //try allocate
            auto alloc_exp = TryAllocate(res_type, req, flags, device, dl, alc, calc);
            if(alloc_exp)
                return *alloc_exp;

            return alloc_exp.error();
        }

        if(!(flags & AllocationFlags::CreateOnExistedPools))
            if(memory_property_flags &
               VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                return VK_ERROR_OUT_OF_DEVICE_MEMORY;
            else
                return VK_ERROR_OUT_OF_HOST_MEMORY;
        else
            return AllocatorResult::MemoryPoolNotEnoughMemory;
    }

    hrs::expected<MemoryTypeAcquireResult, hrs::error>
    MemoryType::TryAcquire(ResourceType res_type,
                           const VkMemoryRequirements& req,
                           hrs::flags<AllocationFlags> flags,
                           VkDevice device,
                           const DeviceLoader& dl,
                           const VkAllocationCallbacks* alc,
                           const std::function<NewPoolSizeCalculator>& calc)
    {
        hrs::assert_true_debug(IsSatisfyIndex(req.memoryTypeBits),
                               "Memory type doesn't satisfy the memory requirements!");
        hrs::assert_true_debug(calc != nullptr,
                               "New pool size calculator function is null pointer!");
        hrs::assert_true_debug(hrs::is_power_of_two(req.alignment),
                               "Alignment is not power of two!");
        hrs::assert_true_debug((flags & AllocationFlags::MapMemory ? IsMappable() : true),
                               "Memory type is considered not to be mapped!");

        //ignore AllocateSeparatePool and CreateOnExistedPools!
        hrs::mem_req<VkDeviceSize> mem_req(req.size, req.alignment);
        MemoryPoolType mem_type = MemoryPool::ToMemoryPoolType(res_type);

        //first -> strict
        auto acq_exp = acquire_existed(mem_type, res_type, flags, mem_req, device, dl);
        if(acq_exp)
            return *acq_exp;

        //second -> none
        acq_exp = acquire_existed(MemoryPoolType::None, res_type, flags, mem_req, device, dl);
        if(acq_exp)
            return *acq_exp;

        //third -> mixed if allowed
        if(flags & AllocationFlags::AllowPlaceWithMixedResources)
        {
            acq_exp = acquire_existed(mem_type, res_type, flags, mem_req, device, dl);
            if(acq_exp)
                return *acq_exp;
        }

        return AllocatorResult::MemoryPoolNotEnoughMemory;
    }

    hrs::expected<MemoryTypeAcquireResult, hrs::error>
    MemoryType::TryAllocate(ResourceType res_type,
                            const VkMemoryRequirements& req,
                            hrs::flags<AllocationFlags> flags,
                            VkDevice device,
                            const DeviceLoader& dl,
                            const VkAllocationCallbacks* alc,
                            const std::function<NewPoolSizeCalculator>& calc)
    {
        hrs::assert_true_debug(IsSatisfyIndex(req.memoryTypeBits),
                               "Memory type doesn't satisfy the memory requirements!");
        hrs::assert_true_debug(calc != nullptr,
                               "New pool size calculator function is null pointer!");
        hrs::assert_true_debug(hrs::is_power_of_two(req.alignment),
                               "Alignment is not power of two!");
        hrs::assert_true_debug((flags & AllocationFlags::MapMemory ? IsMappable() : true),
                               "Memory type is considered not to be mapped!");

        //ignore AllocateSeparatePool and CreateOnExistedPools!
        hrs::mem_req<VkDeviceSize> mem_req(req.size, req.alignment);

        auto blk_exp = allocate_pool_and_acquire(res_type, mem_req, flags, device, dl, alc, calc);

        if(blk_exp)
            return *blk_exp;

        if(memory_property_flags & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            return VK_ERROR_OUT_OF_DEVICE_MEMORY;
        else
            return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    void MemoryType::Release(ResourceType res_type,
                             const MemoryTypeAcquireResult& mtar,
                             MemoryPoolOnEmptyPolicy policy,
                             VkDevice device,
                             const DeviceLoader& dl,
                             const VkAllocationCallbacks* alc)
    {
        hrs::assert_true_debug(
            hrs::is_iterator_part_of_range_debug(lists.GetPools(mtar.pool->GetType()), mtar.pool),
            "Passed pool isn't a apart of this memory type!");

        MemoryPoolType prev_type = mtar.pool->GetType();
        mtar.pool->Release(res_type, mtar.block);
        lists.Rearrange(prev_type, mtar.pool);

        if(policy == MemoryPoolOnEmptyPolicy::Free)
        {
            if(mtar.pool->IsEmpty())
                lists.Release(mtar.pool, device, dl, alc);
        }
    }

    hrs::expected<MemoryTypeAcquireResult, hrs::error>
    MemoryType::acquire_existed(MemoryPoolType pool_type,
                                ResourceType res_type,
                                hrs::flags<AllocationFlags> flags,
                                const hrs::mem_req<VkDeviceSize>& mem_req,
                                VkDevice device,
                                const DeviceLoader& dl)
    {
        for(auto pool_it = lists.GetPools(pool_type).begin();
            pool_it != lists.GetPools(pool_type).end();
            pool_it++)
        {
            auto acq_exp = pool_it->Acquire(res_type, mem_req);
            if(acq_exp)
            {
                if(flags & AllocationFlags::MapMemory)
                {
                    if(!pool_it->GetMemory().IsMapped())
                    {
                        VkResult res = pool_it->GetMemory().MapMemory(device, dl);
                        if(res != VK_SUCCESS)
                            continue;
                    }
                }
                return MemoryTypeAcquireResult(*acq_exp, pool_it);
            }
        }

        return AllocatorResult::MemoryPoolNotEnoughMemory;
    }

    hrs::expected<MemoryTypeAcquireResult, hrs::error>
    MemoryType::allocate_pool_and_acquire(ResourceType res_type,
                                          const hrs::mem_req<VkDeviceSize>& req,
                                          hrs::flags<AllocationFlags> flags,
                                          VkDevice device,
                                          const DeviceLoader& dl,
                                          const VkAllocationCallbacks* alc,
                                          const std::function<NewPoolSizeCalculator>& calc)
    {
        bool initial = true;
        VkDeviceSize previous_size = 0;
        while(true)
        {
            VkDeviceSize tmp_previous_size = calc(initial, previous_size, req.size, *this);
            if(tmp_previous_size == previous_size)
            {
                if(memory_property_flags &
                   VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                    return VK_ERROR_OUT_OF_DEVICE_MEMORY;
                else
                    return VK_ERROR_OUT_OF_HOST_MEMORY;
            }

            auto pool_exp =
                MemoryPool::Create(device,
                                   tmp_previous_size,
                                   index,
                                   static_cast<bool>(flags & AllocationFlags::MapMemory),
                                   buffer_image_granularity,
                                   dl,
                                   alc);
            if(pool_exp)
            {
                auto acq_opt = pool_exp->Acquire(res_type, req);
                hrs::assert_true_debug(acq_opt.has_value(),
                                       "Acquisition must be happened due to the prerequisities!");

                auto it = lists.Insert(std::move(*pool_exp));
                return MemoryTypeAcquireResult(*acq_opt, it);
            }
            initial = false;
            previous_size = tmp_previous_size;
        }
    }
};
