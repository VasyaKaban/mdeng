#include "MemoryPool.h"
#include "../Context/DeviceLoader.h"
#include "AllocatorResult.h"

namespace FireLand
{
    MemoryPool::MemoryPool(VkDeviceSize _buffer_image_granularity,
                           Memory&& _memory,
                           hrs::sized_free_block_chain<VkDeviceSize>&& _free_blocks) noexcept
        : buffer_image_granularity(_buffer_image_granularity),
          non_linear_object_count(0),
          linear_object_count(0),
          memory(std::move(_memory)),
          free_blocks(std::move(_free_blocks))
    {}

    MemoryPool::MemoryPool() noexcept
        : buffer_image_granularity(1),
          non_linear_object_count(0),
          linear_object_count(0)
    {}

    MemoryPool::MemoryPool(MemoryPool&& pool) noexcept
        : buffer_image_granularity(std::exchange(pool.buffer_image_granularity, 1)),
          non_linear_object_count(std::exchange(pool.non_linear_object_count, 0)),
          linear_object_count(std::exchange(pool.linear_object_count, 0)),
          memory(std::move(pool.memory)),
          free_blocks(std::move(pool.free_blocks))
    {}

    MemoryPoolType MemoryPool::ToMemoryPoolType(ResourceType res_type) noexcept
    {
        return (res_type == ResourceType::Linear ? MemoryPoolType::Linear :
                                                   MemoryPoolType::NonLinear);
    }

    hrs::expected<MemoryPool, VkResult> MemoryPool::Create(VkDevice device,
                                                           VkDeviceSize size,
                                                           std::uint32_t memory_type_index,
                                                           bool map_memory,
                                                           VkDeviceSize _buffer_image_granularity,
                                                           const DeviceLoader& dl,
                                                           const VkAllocationCallbacks* alc)
    {
        hrs::assert_true_debug(device != VK_NULL_HANDLE, "Device isn't created yet!");
        hrs::assert_true_debug(hrs::is_power_of_two(_buffer_image_granularity),
                               "Buffer image granularity: {} is not power of two!",
                               _buffer_image_granularity);

        if(size == 0)
            return MemoryPool(_buffer_image_granularity,
                              Memory{},
                              hrs::sized_free_block_chain<VkDeviceSize>(0, 0));

        const VkMemoryAllocateInfo info{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                                        .pNext = nullptr,
                                        .allocationSize = size,
                                        .memoryTypeIndex = memory_type_index};

        VkDeviceMemory _memory;
        VkResult res = dl.vkAllocateMemory(device, &info, alc, &_memory);
        if(res != VK_SUCCESS)
            return res;

        Memory memory_obj(_memory, size, nullptr);
        if(map_memory)
        {
            res = memory_obj.MapMemory(device, dl);
            if(res != VK_SUCCESS)
            {
                memory_obj.Free(device, dl, alc);
                return res;
            }
        }

        return MemoryPool(_buffer_image_granularity,
                          std::move(memory_obj),
                          hrs::sized_free_block_chain<VkDeviceSize>(size, 0));
    }

    bool MemoryPool::IsCreated() const noexcept
    {
        return memory.IsAllocated();
    }

    void MemoryPool::Destroy(VkDevice device,
                             const DeviceLoader& dl,
                             const VkAllocationCallbacks* alc) noexcept
    {
        hrs::assert_true_debug(device != VK_NULL_HANDLE, "Device isn't created yet!");

        if(!IsCreated())
            return;

        memory.Free(device, dl, alc);
        free_blocks.clear();
        linear_object_count = 0;
        non_linear_object_count = 0;
    }

    Memory& MemoryPool::GetMemory() noexcept
    {
        return memory;
    }

    const Memory& MemoryPool::GetMemory() const noexcept
    {
        return memory;
    }

    VkDeviceSize MemoryPool::GetGranularity() const noexcept
    {
        return buffer_image_granularity;
    }

    MemoryPoolType MemoryPool::GetType() const noexcept
    {
        if(IsGranularityFree())
            return MemoryPoolType::None;

        if(linear_object_count == 0 && non_linear_object_count == 0)
            return MemoryPoolType::None;
        else if(linear_object_count != 0 && non_linear_object_count == 0)
            return MemoryPoolType::Linear;
        else if(linear_object_count == 0 && non_linear_object_count != 0)
            return MemoryPoolType::NonLinear;
        else
            return MemoryPoolType::Mixed;
    }

    bool MemoryPool::IsGranularityFree() const noexcept
    {
        return buffer_image_granularity == 1;
    }

    bool MemoryPool::IsEmpty() const noexcept
    {
        return free_blocks.is_empty();
    }

    std::size_t MemoryPool::GetNonLinearObjectCount() const noexcept
    {
        return non_linear_object_count;
    }

    std::size_t MemoryPool::GetLinearObjectCount() const noexcept
    {
        return linear_object_count;
    }

    hrs::expected<std::byte*, VkResult> MemoryPool::MapMemory(VkDevice device,
                                                              const DeviceLoader& dl) noexcept
    {
        hrs::assert_true_debug(IsCreated(), "Memory pool isn't created yet!");
        hrs::assert_true_debug(device != VK_NULL_HANDLE, "Device isn't created yet!");

        VkResult res = memory.MapMemory(device, dl);
        if(res != VK_SUCCESS)
            return res;

        return memory.GetMapPtr();
    }

    hrs::expected<hrs::block<VkDeviceSize>, AllocatorResult>
    MemoryPool::Acquire(ResourceType res_type, const hrs::mem_req<VkDeviceSize>& req)
    {
        hrs::assert_true_debug(IsCreated(), "Memory pool isn't created yet!");

        auto blk_opt = acquire_block_based_on_granularity(res_type, req);
        if(!blk_opt)
            return {AllocatorResult::MemoryPoolNotEnoughMemory};

        inc_count(res_type);
        return blk_opt.value();
    }

    void MemoryPool::Release(ResourceType res_type, const hrs::block<VkDeviceSize>& blk) noexcept
    {
        hrs::assert_true_debug(IsCreated(), "Memory pool isn't created yet!");
        free_blocks.release(blk);
        dec_count(res_type);
    }

    void MemoryPool::inc_count(ResourceType res_type) noexcept
    {
        (res_type == ResourceType::Linear ? linear_object_count : non_linear_object_count)++;
    }

    void MemoryPool::dec_count(ResourceType res_type) noexcept
    {
        std::size_t& object_counter =
            (res_type == ResourceType::Linear ? linear_object_count : non_linear_object_count);
        hrs::assert_true_debug(object_counter != 0,
                               "Release operation with resource cannot be performed"
                               " because counter of target resource type objects is already "
                               "zero!");
        object_counter--;
    }

    std::optional<hrs::block<VkDeviceSize>>
    MemoryPool::acquire_block_based_on_granularity(ResourceType res_type,
                                                   hrs::mem_req<VkDeviceSize> req)
    {
        MemoryPoolType type = GetType();
        const bool can_be_acquired_without_granularity_use =
            (IsGranularityFree() || type == MemoryPoolType::None ||
             (type == ToMemoryPoolType(res_type)));

        if(can_be_acquired_without_granularity_use)
            return free_blocks.acquire(req.size, req.alignment);

        req.alignment = std::max(req.alignment, buffer_image_granularity);
        VkDeviceSize upper_bound_size =
            hrs::round_up_size_to_alignment(req.size, buffer_image_granularity);

        /*We use upper_bound_size because we must to find next bound of page and
		 existed resources cannot alias with new resource

		 Ex: assume that granularity is 4 bytes

		 new resource: size = 9, alignment = 2
		 granularity_fixed_alignment = 4
		 granularity_fixed_size = 12

		   nnnnnnnnn -> resource without granularity
		 p***p***p***p***p***p
		 rrr           rrrrrr--+
					   |	   |
			 nnnnnnnnn000------+--- aliasing(we don't know type of existed resource
					   |_______|		and it's also must be placed with respect to granularity,
										even if the whole new resource fits the free space!)
		 */
        for(auto hint_it = free_blocks.begin(); hint_it != free_blocks.end(); hint_it++)
            if(free_blocks.is_block_can_be_placed(hint_it, upper_bound_size, req.alignment))
                return free_blocks.acquire_by_hint(hint_it, {req.size, req.alignment});

        return {};
    }
};
