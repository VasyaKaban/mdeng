#pragma once

#include "../Vulkan/VulkanInclude.h"
#include "MemoryPoolLists.h"
#include "hrs/block.hpp"
#include "hrs/error.hpp"
#include "hrs/expected.hpp"
#include "hrs/flags.hpp"
#include "hrs/non_creatable.hpp"
#include <cstdint>
#include <functional>
#include <list>

namespace FireLand
{
    class MemoryPool;
    enum class ResourceType;

    enum class MemoryPoolOnEmptyPolicy
    {
        Keep,
        Free,
    };

    enum class AllocationFlags
    {
        AllowPlaceWithMixedResources = 1 << 0,
        AllocateSeparatePool = 1 << 1,
        MapMemory = 1 << 2,
        CreateOnExistedPools = 1 << 3
    };

    enum class MemoryTypeSatisfyOp
    {
        Any,
        Only
    };

    struct MemoryTypeAcquireResult
    {
        hrs::block<VkDeviceSize> block;
        MemoryPoolLists::Iterator pool;

        MemoryTypeAcquireResult(const hrs::block<VkDeviceSize>& _block = {},
                                MemoryPoolLists::Iterator _pool = {}) noexcept
            : block(_block),
              pool(_pool)
        {}
        MemoryTypeAcquireResult(const MemoryTypeAcquireResult&) = default;
        MemoryTypeAcquireResult& operator=(const MemoryTypeAcquireResult&) = default;
    };

    class MemoryType;

    //return value must be greater or equal to requested_size if it's supposed to be a good allocation size
    //otherwise it's supposed to be an allocation failure
    using NewPoolSizeCalculator = VkDeviceSize(
        bool, /*initial -> true if it first call to this fucntion*/
        VkDeviceSize /*previous_size -> returned value for next calls*/,
        VkDeviceSize /*requested_size -> requested size for allocation(passes on first call)*/
        ,
        const MemoryType& /*mem_type -> memory type where pool wiil be placed*/);

    class MemoryType : public hrs::non_copyable, public hrs::non_move_assignable
    {
    public:
        using PoolContainer = std::list<MemoryPool>;

        static VkDeviceSize DefaultNewPoolSizeCalculator(bool initial,
                                                         VkDeviceSize previous_size,
                                                         VkDeviceSize requested_size,
                                                         const MemoryType& mem_type) noexcept;

        MemoryType(VkMemoryHeap _heap,
                   VkMemoryPropertyFlags _memory_property_flags,
                   std::uint32_t _index,
                   VkDeviceSize _buffer_image_granularity) noexcept;

        ~MemoryType() = default;
        MemoryType(MemoryType&& mem_type) noexcept;

        void
        Destroy(VkDevice device, const DeviceLoader& dl, const VkAllocationCallbacks* alc) noexcept;

        bool IsSatisfy(MemoryTypeSatisfyOp satisfy, VkMemoryPropertyFlags props) const noexcept;
        bool IsSatisfyIndex(std::uint32_t memory_type_bits) const noexcept;
        bool IsMappable() const noexcept;
        bool IsEmpty() const noexcept;

        std::uint32_t GetMemoryTypeIndex() const noexcept;
        std::uint32_t GetMemoryTypeIndexMask() const noexcept;

        hrs::expected<MemoryTypeAcquireResult, hrs::error>
        Allocate(ResourceType res_type,
                 const VkMemoryRequirements& req,
                 hrs::flags<AllocationFlags> flags,
                 VkDevice device,
                 const DeviceLoader& dl,
                 const VkAllocationCallbacks* alc,
                 const std::function<NewPoolSizeCalculator>& calc = DefaultNewPoolSizeCalculator);

        hrs::expected<MemoryTypeAcquireResult, hrs::error>
        TryAcquire(ResourceType res_type,
                   const VkMemoryRequirements& req,
                   hrs::flags<AllocationFlags> flags,
                   VkDevice device,
                   const DeviceLoader& dl,
                   const VkAllocationCallbacks* alc,
                   const std::function<NewPoolSizeCalculator>& calc = DefaultNewPoolSizeCalculator);

        hrs::expected<MemoryTypeAcquireResult, hrs::error> TryAllocate(
            ResourceType res_type,
            const VkMemoryRequirements& req,
            hrs::flags<AllocationFlags> flags,
            VkDevice device,
            const DeviceLoader& dl,
            const VkAllocationCallbacks* alc,
            const std::function<NewPoolSizeCalculator>& calc = DefaultNewPoolSizeCalculator);

        void Release(ResourceType res_type,
                     const MemoryTypeAcquireResult& mtar,
                     MemoryPoolOnEmptyPolicy policy,
                     VkDevice device,
                     const DeviceLoader& dl,
                     const VkAllocationCallbacks* alc);
    private:
        hrs::expected<MemoryTypeAcquireResult, hrs::error>
        acquire_existed(MemoryPoolType pool_type,
                        ResourceType res_type,
                        hrs::flags<AllocationFlags> flags,
                        const hrs::mem_req<VkDeviceSize>& mem_req,
                        VkDevice device,
                        const DeviceLoader& dl);

        hrs::expected<MemoryTypeAcquireResult, hrs::error>
        allocate_pool_and_acquire(ResourceType res_type,
                                  const hrs::mem_req<VkDeviceSize>& req,
                                  hrs::flags<AllocationFlags> flags,
                                  VkDevice device,
                                  const DeviceLoader& dl,
                                  const VkAllocationCallbacks* alc,
                                  const std::function<NewPoolSizeCalculator>& calc);
    private:
        VkMemoryHeap heap;
        VkMemoryPropertyFlags memory_property_flags;
        std::uint32_t index;
        VkDeviceSize buffer_image_granularity;
        MemoryPoolLists lists;
    };
};
